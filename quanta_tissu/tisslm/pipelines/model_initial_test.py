import os
import sys
import numpy as np
import glob

# --- System Path Setup ---
# Ensure the project root is in sys.path for module imports
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
if project_root not in sys.path:
    sys.path.insert(0, project_root)

# --- Core Imports ---
from quanta_tissu.tisslm.core.bpe_trainer import BPETokenizer
from quanta_tissu.tisslm.core.tokenizer import Tokenizer
from quanta_tissu.tisslm.core.model import QuantaTissu
from quanta_tissu.tisslm.core.loss import CrossEntropyLoss
from quanta_tissu.tisslm.core.optimizer import AdamW
from quanta_tissu.tisslm.core.data import Dataset, load_corpus
from quanta_tissu.tisslm.core.utils import save_checkpoint
from quanta_tissu.tisslm.core.generate_text import generate_text as generate_text_from_model
from quanta_tissu.tisslm.config import model_config, training_config, system_config, tokenizer_config

def main():
    """
    Runs the full pipeline: train tokenizer, create checkpoint, and generate text.
    """
    print("--- Starting Model Initial Test ---")

    # --- Configuration ---
    models_dir = os.path.join(system_config["_project_root"], "models")
    corpus_dir = os.path.join(system_config["_project_root"], "corpus")
    checkpoint_dir = os.path.join(models_dir, "checkpoints")
    tokenizer_prefix = system_config["bpe_tokenizer_prefix"] # Use default prefix

    # Ensure directories exist
    os.makedirs(models_dir, exist_ok=True)
    os.makedirs(checkpoint_dir, exist_ok=True)

    try:
        # --- Step 1: Train BPE Tokenizer ---
        print("\n[Step 1/3] Training BPE Tokenizer...")

        # Load corpus text (optimized to use only the first file)
        print(" -> Loading corpus from first .txt file found...")
        full_text = ""
        txt_files = glob.glob(os.path.join(corpus_dir, "*.txt"))
        if not txt_files:
            raise FileNotFoundError(f"No .txt files found in corpus directory: {corpus_dir}")

        with open(txt_files[0], "r", encoding="utf-8", errors="replace") as f:
            full_text = f.read()
        print(f" -> Corpus loaded from: {txt_files[0]}")

        if not full_text.strip():
            raise ValueError("Corpus is empty.")

        # Train and save the BPE tokenizer
        bpe_tokenizer = BPETokenizer()
        bpe_tokenizer.train(full_text, vocab_size=256, verbose=False) # Reduced vocab_size for speed
        bpe_tokenizer.save(tokenizer_prefix)
        print(f"Tokenizer trained and saved to prefix: {tokenizer_prefix}")

        # --- Step 2: Create Model Checkpoint ---
        print("\n[Step 2/3] Creating Model Checkpoint...")

        # Initialize the main tokenizer which loads from the saved files
        print(" -> Initializing tokenizer from saved files...")
        tokenizer = Tokenizer()
        
        # Initialize model components
        print(" -> Initializing model, loss function, and optimizer...")
        model_config["vocab_size"] = tokenizer.get_vocab_size()
        model = QuantaTissu(model_config)
        loss_fn = CrossEntropyLoss()
        optimizer = AdamW(model.parameters(), lr=training_config["learning_rate"], weight_decay=training_config["weight_decay"])

        # Load data
        print(" -> Loading and tokenizing corpus for training...")
        token_ids = load_corpus(corpus_dir, tokenizer)
        dataset = Dataset(token_ids, batch_size=4, seq_len=tokenizer_config["max_len"]) # Use smaller batch for test

        # Training loop for a single step
        print(" -> Preparing dataset for training...")
        x_batch, y_batch = next(iter(dataset)) # Get one batch

        # Forward, backward, and optimization
        print(" -> Running a single forward/backward pass...")
        logits = model.forward(x_batch)
        loss = loss_fn.forward(logits, y_batch)
        d_logits = loss_fn.backward()
        model.backward(d_logits)
        optimizer.step()
        optimizer.zero_grad()

        print(f"Single training step completed. Loss: {loss:.4f}")

        # Save checkpoint
        checkpoint_path = save_checkpoint(model, optimizer, epoch=0, step=1, checkpoint_dir=checkpoint_dir)
        print(f"Checkpoint saved at: {checkpoint_path}")

        # --- Step 3: Generate Text ---
        print("\n[Step 3/3] Generating Text from Checkpoint...")

        # Re-initialize a clean model to load weights into
        print(" -> Initializing new model for generation...")
        generation_model = QuantaTissu(model_config)
        print(" -> Loading weights from checkpoint...")
        generation_model.load_weights(checkpoint_path)
        print(f"Model weights loaded from: {checkpoint_path}")

        # Generate text
        prompt = "This is a test of the model's ability to generate text."
        print(f"Prompt: '{prompt}'")

        generated_text = generate_text_from_model(
            model=generation_model,
            tokenizer=tokenizer,
            prompt=prompt,
            length=50,
            method="nucleus",
            temperature=0.8,
            top_k=20,
            top_p=0.9
        )

        print("\n--- Generated Text ---")
        print(generated_text)
        print("----------------------")

        print("\n--- Model initial test completed successfully! ---")

    except Exception as e:
        import traceback
        print(f"\n--- An error occurred during the test: {e} ---", file=sys.stderr)
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()
