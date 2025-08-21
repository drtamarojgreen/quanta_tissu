import sys
import os

# Add the project root to sys.path for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..')) # Adjust '..' count based on file location relative to project root
sys.path.insert(0, project_root)

import argparse
import numpy as np
import logging

from .tokenizer import Tokenizer # Changed from tokenizers import Tokenizer as HFTokenizer
from .model import QuantaTissu
from .loss import CrossEntropyLoss
from .optimizer import AdamW
from .data import Dataset, load_corpus
from ..config import model_config, training_config, system_config, tokenizer_config
from .scheduler import CosineDecayWithWarmup
from .utils import save_checkpoint, load_checkpoint

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def main():
    parser = argparse.ArgumentParser(description="Train the QuantaTissu model.")
    parser.add_argument("--corpus_path", type=str, default=os.path.join(system_config["_project_root"], "corpus"), help="Path to the training corpus directory.")
    parser.add_argument("--epochs", type=int, default=training_config["num_epochs"], help="Number of training epochs.")
    parser.add_argument("--batch_size", type=int, default=training_config["batch_size"], help="Batch size for training.")
    parser.add_argument("--seq_len", type=int, default=tokenizer_config["max_len"], help="Sequence length for training.")
    parser.add_argument("--lr", type=float, default=training_config["learning_rate"], help="Maximum learning rate.")
    parser.add_argument("--weight_decay", type=float, default=training_config["weight_decay"], help="Weight decay for AdamW.")
    parser.add_argument("--warmup_steps", type=int, default=50, help="Number of warmup steps for the scheduler.")
    parser.add_argument("--max_grad_norm", type=float, default=1.0, help="Maximum value for gradient clipping.")
    parser.add_argument("--checkpoint_dir", type=str, default=os.path.join(system_config["_project_root"], "checkpoints"), help="Directory to save checkpoints.")
    parser.add_argument("--resume_from", type=str, default=None, help="Path to a checkpoint to resume training from.")
    # Removed --tokenizer_path argument
    parser.add_argument("--save_every", type=int, default=100, help="Save a checkpoint every N steps.")
    parser.add_argument("--keep_checkpoints", type=int, default=-1, help="Number of recent checkpoints to keep. Use -1 to keep all.")

    args = parser.parse_args()

    logging.info("--- Initializing training ---")

    # 1. Components Initialization
    try:
        tokenizer = Tokenizer() # Use our custom Tokenizer
    except Exception as e:
        logging.error(f"Could not initialize tokenizer. Error: {e}") # Updated error message
        return

    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)
    loss_fn = CrossEntropyLoss()
    optimizer = AdamW(model.parameters(), lr=args.lr, weight_decay=args.weight_decay)

    # Load corpus and create dataset
    logging.info(f"Loading corpus from: {args.corpus_path}")
    token_ids = load_corpus(args.corpus_path, tokenizer)
    logging.info(f"Corpus loaded. Total tokens: {len(token_ids)}")

    dataset = Dataset(token_ids, args.batch_size, args.seq_len)
    total_steps = args.epochs * len(dataset)

    scheduler = CosineDecayWithWarmup(optimizer, warmup_steps=args.warmup_steps, total_steps=total_steps, max_lr=args.lr)

    # 2. Load from checkpoint if specified
    start_epoch = 0
    step = 0
    if args.resume_from:
        start_epoch, step = load_checkpoint(model, optimizer, args.resume_from)
        # Manually set scheduler's optimizer LR to the loaded state
        scheduler.optimizer.lr = optimizer.lr

    logging.info(f"Model: QuantaTissu with {len(model.parameters())} parameter groups.")
    logging.info(f"Dataset: {len(dataset)} batches of size {args.batch_size}x{args.seq_len}.")
    logging.info(f"Total training steps: {total_steps}.")
    logging.info(f"Starting from epoch {start_epoch+1}, step {step}.")

    # 3. Training Loop
    for epoch in range(start_epoch, args.epochs):
        logging.info(f"\n--- Epoch {epoch+1}/{args.epochs} ---")
        for x_batch, y_batch in dataset:
            # Forward pass
            logits = model.forward(x_batch)

            # Calculate loss
            loss = loss_fn.forward(logits, y_batch)

            # Backward pass
            d_logits = loss_fn.backward()
            model.backward(d_logits)

            # Gradient Clipping
            params = model.parameters()
            grads = [p.grad for p in params if p.grad is not None]
            global_norm = np.sqrt(sum(np.sum(g**2) for g in grads))

            if global_norm > args.max_grad_norm:
                clip_coef = args.max_grad_norm / (global_norm + 1e-6)
                for g in grads:
                    g *= clip_coef

            # Update weights
            optimizer.step()

            # Update learning rate
            current_lr = scheduler.step(step)

            # Reset gradients for next step
            optimizer.zero_grad()

            if step % 10 == 0:
                logging.info(f"Step {step}, Loss: {loss:.4f}, LR: {current_lr:.6f}, Grad Norm: {global_norm:.4f}")

            # Save checkpoint periodically
            if step > 0 and step % args.save_every == 0:
                save_checkpoint(model, optimizer, epoch, step, args.checkpoint_dir, keep_last=args.keep_checkpoints)

            step += 1

    logging.info("\n--- Training complete ---")
    # Save final model
    save_checkpoint(model, optimizer, args.epochs - 1, step, args.checkpoint_dir, keep_last=args.keep_checkpoints)


if __name__ == "__main__":
    main()