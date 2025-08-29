import sys
import os
import argparse
import logging

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.loss import CrossEntropyLoss
from quanta_tissu.tisslm.core.optimizer import AdamW
from quanta_tissu.tisslm.core.data import Dataset, load_corpus
from quanta_tissu.tisslm.core.scheduler import CosineDecayWithWarmup
from quanta_tissu.tisslm.core.training.trainer import Trainer
from quanta_tissu.tisslm.config import model_config, training_config, system_config, tokenizer_config

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def main():
    parser = argparse.ArgumentParser(description="Train the QuantaTissu model using the new Trainer.")
    # Arguments remain the same, as they define the configuration
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
    parser.add_argument("--save_every", type=int, default=100, help="Save a checkpoint every N steps.")
    parser.add_argument("--keep_checkpoints", type=int, default=-1, help="Number of recent checkpoints to keep. Use -1 to keep all.")
    parser.add_argument("--tokenizer_path", type=str, default=os.path.join(system_config["_project_root"], "models", "tokenizers", "revised_tokenizer"), help="Path to the trained tokenizer files.")

    args = parser.parse_args()

    logging.info("--- Initializing training components ---")

    try:
        tokenizer = Tokenizer(tokenizer_path=args.tokenizer_path)
    except Exception as e:
        logging.error(f"Could not initialize tokenizer. Error: {e}")
        return

    # Prepare model and data
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    token_ids = load_corpus(args.corpus_path, tokenizer)
    dataset = Dataset(token_ids, args.batch_size, args.seq_len)

    # The QuantaTissu facade now only handles model setup and weight loading
    model = QuantaTissu(model_config)

    # Initialize optimizer and scheduler
    optimizer = AdamW(model.parameters(), lr=args.lr, weight_decay=args.weight_decay)
    total_steps = args.epochs * len(dataset)
    scheduler = CosineDecayWithWarmup(optimizer, warmup_steps=args.warmup_steps, total_steps=total_steps, max_lr=args.lr)

    loss_fn = CrossEntropyLoss()

    # Consolidate trainer configuration from args
    trainer_config = {
        "epochs": args.epochs,
        "max_grad_norm": args.max_grad_norm,
        "checkpoint_dir": args.checkpoint_dir,
        "save_every": args.save_every,
        "resume_from": args.resume_from,
        "keep_checkpoints": args.keep_checkpoints
    }

    logging.info("--- Initializing Trainer ---")

    # Instantiate the Trainer with all components
    trainer = Trainer(
        model=model,
        optimizer=optimizer,
        loss_fn=loss_fn,
        scheduler=scheduler,
        dataset=dataset,
        config=trainer_config
    )

    # The entire training process is now encapsulated in the trainer.train() call
    logging.info("--- Starting Training ---")
    trainer.train()
    logging.info("--- Training Finished ---")


if __name__ == "__main__":
    main()