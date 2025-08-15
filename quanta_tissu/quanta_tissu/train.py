import os
import numpy as np
from tokenizers import Tokenizer
from tokenizers.models import BPE
from tokenizers.trainers import BpeTrainer
from tokenizers.pre_tokenizers import Whitespace

from config import tokenizer_config, training_config, model_config, system_config
from data import TissuDataset, TissuDataLoader
from model import Transformer
from loss import CrossEntropyLoss
from optimizer import Adam


def train():
    """
    Main training loop for the Tiss LLM.
    """
    # 1. Load the data
    corpus_path = os.path.join(os.path.dirname(__file__), '..', '..', 'corpus', 'full_corpus.txt')
    with open(corpus_path, "r", encoding="utf-8") as f:
        text = f.read()

    # 2. Train the tokenizer
    tokenizer = Tokenizer(BPE(unk_token="<unk>"))
    tokenizer.pre_tokenizer = Whitespace()
    trainer = BpeTrainer(special_tokens=["<unk>", "<pad>"], vocab_size=model_config["vocab_size"])
    tokenizer.train_from_iterator([text], trainer=trainer)
    tokenizer_path = os.path.join(os.path.dirname(__file__), '..', '..', 'models', 'tokenizer.json')
    tokenizer.save(tokenizer_path)

    print(f"Tokenizer trained and saved to {tokenizer_path}")

    # 3. Tokenize the data
    tokenized_data = tokenizer.encode(text).ids

    # 4. Create a data loader
    dataset = TissuDataset(tokenized_data, tokenizer_config["max_len"])
    dataloader = TissuDataLoader(dataset, training_config["batch_size"])

    # 5. Initialize the model
    model = Transformer(model_config)

    # 6. Initialize the optimizer and loss function
    optimizer = Adam(model.params(), learning_rate=training_config["learning_rate"], weight_decay=training_config["weight_decay"])
    loss_fn = CrossEntropyLoss()

    # 7. Train the model
    for epoch in range(training_config["num_epochs"]):
        for i, (x, y) in enumerate(dataloader):
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
    model_path = os.path.join(os.path.dirname(__file__), '..', '..', 'models', 'quanta_tissu.npz')
    np.savez(model_path, **{k: v.data for k, v in model.params().items()})

    print(f"Model trained and saved to {model_path}")

if __name__ == "__main__":
    train()
