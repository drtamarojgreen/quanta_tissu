import sys
import os

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

import argparse
import numpy as np
import logging

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.loss import CrossEntropyLoss
from quanta_tissu.tisslm.core.optimizer import AdamW
from quanta_tissu.tisslm.core.data import Dataset, load_corpus
from quanta_tissu.tisslm.config import model_config, training_config, system_config, tokenizer_config
from quanta_tissu.tisslm.core.scheduler import CosineDecayWithWarmup
from quanta_tissu.tisslm.core.utils import save_checkpoint, load_checkpoint
from quanta_tissu.tisslm.evaluation.training_debugger import inspect_model_parameters, check_gradients
from quanta_tissu.tisslm.core.generate_text import generate_text

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def main():
    parser = argparse.ArgumentParser(description="Train the QuantaTissu model with custom tokenizer and text generation.")
    parser.add_argument("--tokenizer_path", type=str, default="models/trained_tokenizer", help="Path prefix for the tokenizer files, relative to project root.")
    parser.add_argument("--corpus_path", type=str, default=os.path.join(project_root, "corpus"), help="Path to the training corpus directory.")
    parser.add_argument("--epochs", type=int, default=training_config["num_epochs"], help="Number of training epochs.")
    parser.add_argument("--batch_size", type=int, default=training_config["batch_size"], help="Batch size for training.")
    parser.add_argument("--seq_len", type=int, default=tokenizer_config["max_len"], help="Sequence length for training.")
    parser.add_argument("--lr", type=float, default=training_config["learning_rate"], help="Maximum learning rate.")
    parser.add_argument("--weight_decay", type=float, default=training_config["weight_decay"], help="Weight decay for AdamW.")
    parser.add_argument("--warmup_steps", type=int, default=50, help="Number of warmup steps for the scheduler.")
    parser.add_argument("--max_grad_norm", type=float, default=1.0, help="Maximum value for gradient clipping.")
    parser.add_argument("--checkpoint_dir", type=str, default=os.path.join(project_root, "checkpoints"), help="Directory to save checkpoints.")
    parser.add_argument("--resume_from", type=str, default=None, help="Path to a checkpoint to resume training from.")
    parser.add_argument("--save_every", type=int, default=1000, help="Save a checkpoint and generate text every N steps.")
    parser.add_argument("--keep_checkpoints", type=int, default=-1, help="Number of recent checkpoints to keep. Use -1 to keep all.")

    args = parser.parse_args()

    logging.info("--- Initializing training ---")

    # 1. Components Initialization
    try:
        tokenizer_full_path = os.path.normpath(os.path.join(project_root, args.tokenizer_path))
        tokenizer = Tokenizer(tokenizer_path=tokenizer_full_path)
    except Exception as e:
        logging.error(f"Could not initialize tokenizer from path '{tokenizer_full_path}'. Error: {e}")
        return

    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)
    loss_fn = CrossEntropyLoss()
    optimizer = AdamW(model.parameters(), lr=args.lr, weight_decay=args.weight_decay)

    # Load corpus and create dataset
    logging.info(f"Loading corpus from: {args.corpus_path}")
    token_ids = load_corpus(args.corpus_path, tokenizer)
    logging.info(f"Corpus loaded. Total tokens: {len(token_ids)}")

    # Split data into training and validation sets
    split_idx = int(len(token_ids) * 0.9)
    train_token_ids = token_ids[:split_idx]
    val_token_ids = token_ids[split_idx:]

    dataset = Dataset(train_token_ids, args.batch_size, args.seq_len)
    val_dataset = Dataset(val_token_ids, args.batch_size, args.seq_len) # Create validation dataset
    
    total_steps = args.epochs * len(dataset)

    scheduler = CosineDecayWithWarmup(optimizer, warmup_steps=args.warmup_steps, total_steps=total_steps, max_lr=args.lr)

    # 2. Load from checkpoint if specified
    start_epoch = 0
    step = 0
    if args.resume_from:
        start_epoch, step = load_checkpoint(model, optimizer, args.resume_from)
        scheduler.optimizer.lr = optimizer.lr

    logging.info(f"Model: QuantaTissu with {len(model.parameters())} parameter groups.")
    logging.info(f"Dataset: {len(dataset)} batches of size {args.batch_size}x{args.seq_len}.")
    logging.info(f"Total training steps: {total_steps}.")
    logging.info(f"Starting from epoch {start_epoch+1}, step {step}.")

    # 3. Training Loop
    for epoch in range(start_epoch, args.epochs):
        logging.info(f"\n--- Epoch {epoch+1}/{args.epochs} ---")
        for x_batch, y_batch in dataset:
            # The model.forward() returns a tuple (logits, kv_cache).
            # The cache is required for the backward pass.
            logits, cache = model.forward(x_batch)
            loss = loss_fn.forward(logits, y_batch)
            d_logits = loss_fn.backward()
            # The backward pass requires the cache from the forward pass
            # to efficiently compute gradients.
            model.backward(d_logits, cache)

            params = model.parameters()
            grads = [p.grad for p in params if p.grad is not None]
            global_norm = np.sqrt(sum(np.sum(g**2) for g in grads))

            if global_norm > args.max_grad_norm:
                clip_coef = args.max_grad_norm / (global_norm + 1e-6)
                for g in grads:
                    g *= clip_coef

            optimizer.step()
            current_lr = scheduler.step(step)
            optimizer.zero_grad()

            if step % 10 == 0:
                logging.info(f"Step {step}, Loss: {loss:.4f}, LR: {current_lr:.6f}, Grad Norm: {global_norm:.4f}")

            if step > 0 and step % args.save_every == 0:
                checkpoint_path = save_checkpoint(model, optimizer, epoch, step, args.checkpoint_dir, keep_last=args.keep_checkpoints)
                logging.info(f"--- Generating text from checkpoint: {checkpoint_path} ---")
                generated_text = generate_text(
                    model=model,
                    tokenizer=tokenizer,
                    prompt="The meaning of life is",
                    length=50,
                    method="nucleus",
                    temperature=0.8,
                    top_k=20,
                    top_p=0.9
                )
                logging.info(f"Generated text: {generated_text}")

                # --- Live Training Diagnostics ---
                # Inspect parameters and gradients to catch issues like parameter collapse or
                # vanishing/exploding gradients as they happen.
                logging.info("--- Running Live Diagnostics ---")
                inspect_model_parameters(model)
                check_gradients(model)
                logging.info("--- Live Diagnostics Complete ---")

            step += 1

    logging.info("\n--- Training complete ---")
    save_checkpoint(model, optimizer, args.epochs - 1, step, args.checkpoint_dir, keep_last=args.keep_checkpoints)


if __name__ == "__main__":
    main()
