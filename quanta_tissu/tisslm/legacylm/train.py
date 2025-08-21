import os
import numpy as np

from .config import tokenizer_config, training_config, model_config, system_config
from .data import Dataset, load_corpus
from .model import QuantaTissu
from .loss import CrossEntropyLoss
from .optimizer import AdamW


def train():
    """
    Main training loop for the Tiss LLM.
    """
    # 1. Load the data
    corpus_path = os.path.join(os.path.dirname(__file__), '..', '..', 'corpus')
    tokenized_data = load_corpus(corpus_path)

    print(f"Corpus loaded and tokenized. Number of tokens: {len(tokenized_data)}")

    # 4. Create a data loader
    dataset = Dataset(tokenized_data, training_config["batch_size"], tokenizer_config["max_len"])

    # 5. Initialize the model
    # The database connection is not needed for training, so we disable it.
    model = QuantaTissu(model_config, use_db=False)

    # 6. Initialize the optimizer and loss function
    optimizer = AdamW(model.parameters(), lr=training_config["learning_rate"], weight_decay=training_config["weight_decay"])
    loss_fn = CrossEntropyLoss()

    # 7. Train the model
    for epoch in range(training_config["num_epochs"]):
        for i, (x, y) in enumerate(dataset):
            # Forward pass
            logits = model.forward(x)

            # Compute loss
            loss = loss_fn.forward(logits, y)

            # Backward pass
            loss_fn.backward()
            model.backward(loss_fn.d_inputs)

            # Update weights
            optimizer.step()

            if i % 10 == 0:
                print(f"Epoch {epoch+1}/{training_config['num_epochs']}, Step {i}, Loss: {loss}")

    # 8. Save the model
    model_path = system_config["model_save_path"]
    # Create a dictionary of parameters to save
    model_params_to_save = {p.name: p.value for p in model.parameters()}
    np.savez(model_path, **model_params_to_save)

    print(f"Model trained and saved to {model_path}")

if __name__ == "__main__":
    train()