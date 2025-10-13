
import os
import sys
import glob
import time
import numpy as np

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
from quanta_tissu.tisslm.core.generate_text import generate_text
from quanta_tissu.tisslm.config import model_config, training_config, tokenizer_config

# --- Configuration ---
TEST_CORPUS_DIR = os.path.join(project_root, "test_corpus")
TEST_TOKENIZER_DIR = os.path.join(project_root, "test_tokenizer")
TEST_MODEL_DIR = os.path.join(project_root, "test_model")
TOKENIZER_SAVE_PREFIX = os.path.join(TEST_TOKENIZER_DIR, "test_tokenizer")

# --- Custom Trainer for Step-Based Training ---
class CustomTrainer(Trainer):
    """A custom trainer to allow stopping at a max number of steps."""
    def train(self, max_steps):
        start_epoch = 0
        step = 0

        if self.config.get("resume_from"):
            print(f"Resuming training from checkpoint: {self.config['resume_from']}")
            start_epoch, step = load_checkpoint(self.model, self.optimizer, self.config["resume_from"])

        print(f"Starting training, will stop at {max_steps} steps.")

        training_complete = False
        for epoch in range(start_epoch, self.config['epochs']):
            if training_complete: break
            print(f"\n--- Epoch {epoch + 1}/{self.config['epochs']} ---")
            for x_batch, y_batch in self.dataset:
                if step >= max_steps:
                    training_complete = True
                    break

                logits, cache = self.model.forward(x_batch)
                loss = self.loss_fn.forward(logits, y_batch)
                d_logits = self.loss_fn.backward()
                self._backward(d_logits, cache) # Use the Trainer's backward pass

                params = self.model.parameters()
                grads = [p.grad for p in params if p.grad is not None]
                global_norm = np.sqrt(sum(np.sum(g**2) for g in grads))
                if self.config.get("max_grad_norm") and global_norm > self.config["max_grad_norm"]:
                    clip_coef = self.config["max_grad_norm"] / (global_norm + 1e-6)
                    for p in params:
                        if p.grad is not None:
                            p.grad *= clip_coef

                self.optimizer.step()
                current_lr = self.scheduler.step(step) if self.scheduler else self.optimizer.lr
                self.optimizer.zero_grad()

                if step % 10 == 0:
                    print(f"Step {step}, Loss: {loss:.4f}, LR: {current_lr:.6f}")

                if self.config.get("save_every") and step > 0 and step % self.config["save_every"] == 0:
                    save_checkpoint(self.model, self.optimizer, epoch, step, self.config["checkpoint_dir"], keep_last=-1)
                
                step += 1
        
        print("\n--- Training complete ---")
        # Save final checkpoint
        final_step = min(step, max_steps)
        save_checkpoint(self.model, self.optimizer, epoch, final_step, self.config["checkpoint_dir"], keep_last=-1)

def train_model_step():
    """Trains the model using the new tokenizer and corpus."""
    print("\n--- Step 1: Training Model ---")
    os.makedirs(TEST_MODEL_DIR, exist_ok=True)

    tokenizer = Tokenizer(tokenizer_path=TOKENIZER_SAVE_PREFIX)
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    
    token_ids = load_corpus(TEST_CORPUS_DIR, tokenizer)
    dataset = Dataset(token_ids, batch_size=training_config["batch_size"], seq_len=tokenizer_config["max_len"])
    
    model = QuantaTissu(model_config)
    optimizer = AdamW(model.parameters(), lr=training_config["learning_rate"], weight_decay=training_config["weight_decay"])
    scheduler = CosineDecayWithWarmup(optimizer, warmup_steps=50, total_steps=1000, max_lr=training_config["learning_rate"])
    loss_fn = CrossEntropyLoss()

    trainer_config = {
        "epochs": 100,  # Set high, will be stopped by max_steps
        "max_grad_norm": 1.0,
        "checkpoint_dir": TEST_MODEL_DIR,
        "save_every": 200,
    }

    trainer = CustomTrainer(
        model=model,
        optimizer=optimizer,
        loss_fn=loss_fn,
        scheduler=scheduler,
        dataset=dataset,
        config=trainer_config
    )
    
    trainer.train(max_steps=1000)
    print("Model training complete.")

def generate_text_step():
    """Generates text using the final trained model."""
    print("\n--- Step 2: Generating Text ---")
    final_checkpoint_path = os.path.join(TEST_MODEL_DIR, "checkpoint_step_1000.npz")
    if not os.path.exists(final_checkpoint_path):
        raise FileNotFoundError(f"Final checkpoint not found at {final_checkpoint_path}")

    tokenizer = Tokenizer(tokenizer_path=TOKENIZER_SAVE_PREFIX)
    model_config["vocab_size"] = tokenizer.get_vocab_size()
    model = QuantaTissu(model_config)
    model.load_weights(final_checkpoint_path)

    prompt = "The meaning of life is"
    print(f"Generating text with prompt: '{prompt}'")
    
    generated_text = generate_text(
        model=model,
        tokenizer=tokenizer,
        prompt=prompt,
        length=50, # Generate 50 new tokens
        method="nucleus",
        temperature=0.8,
        top_p=0.9,
        top_k=20
    )
    
    print("\n--- Generated Text ---")
    print(generated_text)
    print("----------------------")

def main():
    """Main function to run the half tuning pipeline."""
    # Step 1
    train_model_step()
    
    # Pause
    input("\n*** Model training complete. Press Enter to generate text... ***")
    
    # Step 2
    generate_text_step()
    
    print("\nHalf tuning script finished.")

if __name__ == "__main__":
    main()
