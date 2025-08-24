import os
import glob
import json
import logging
from datetime import datetime
from quanta_tissu.tisslm.core.bpe_trainer import BPETokenizer  # Adjust import path
import torch
from torch import nn, optim
from torch.utils.data import DataLoader, Dataset

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# ---------- Corpus Loader ----------
def load_corpus(corpus_path):
    """Concatenate all .txt files in the corpus into a single string."""
    files = glob.glob(os.path.join(corpus_path, "*.txt"))
    corpus = ""
    for file_path in files:
        try:
            with open(file_path, "r", encoding="utf-8", errors="replace") as f:
                corpus += f.read() + "\n"
        except Exception as e:
            logging.warning(f"Skipping {file_path}: {e}")
    logging.info(f"Loaded {len(files)} files from corpus.")
    return corpus

# ---------- Dataset ----------
class TokenDataset(Dataset):
    """Dataset of token ID sequences for training."""
    def __init__(self, token_ids, seq_len=128):
        self.data = token_ids
        self.seq_len = seq_len

    def __len__(self):
        return max(0, len(self.data) - self.seq_len)

    def __getitem__(self, idx):
        x = self.data[idx:idx+self.seq_len]
        y = self.data[idx+1:idx+self.seq_len+1]
        return torch.tensor(x, dtype=torch.long), torch.tensor(y, dtype=torch.long)

# ---------- Simple Model ----------
class SimpleBPEModel(nn.Module):
    def __init__(self, vocab_size, emb_size=128, hidden_size=256):
        super().__init__()
        self.embed = nn.Embedding(vocab_size, emb_size)
        self.rnn = nn.GRU(emb_size, hidden_size, batch_first=True)
        self.fc = nn.Linear(hidden_size, vocab_size)

    def forward(self, x, hidden=None):
        x = self.embed(x)
        out, hidden = self.rnn(x, hidden)
        logits = self.fc(out)
        return logits, hidden

# ---------- Training Loop ----------
def train_model(tokenizer, corpus, save_dir, seq_len=128, batch_size=32, epochs=5, lr=1e-3, checkpoint_every=1000):
    os.makedirs(save_dir, exist_ok=True)
    token_ids = tokenizer.encode(corpus)
    vocab_size = len(tokenizer.vocab)
    dataset = TokenDataset(token_ids, seq_len)
    dataloader = DataLoader(dataset, batch_size=batch_size, shuffle=True)

    model = SimpleBPEModel(vocab_size)
    optimizer = optim.Adam(model.parameters(), lr=lr)
    criterion = nn.CrossEntropyLoss()

    step = 0
    for epoch in range(epochs):
        for x, y in dataloader:
            optimizer.zero_grad()
            logits, _ = model(x)
            loss = criterion(logits.view(-1, vocab_size), y.view(-1))
            loss.backward()
            optimizer.step()

            step += 1
            if step % 100 == 0:
                logging.info(f"Epoch {epoch+1}, Step {step}, Loss: {loss.item():.4f}")

            if step % checkpoint_every == 0:
                checkpoint_path = os.path.join(save_dir, f"checkpoint_step_{step}.pt")
                torch.save(model.state_dict(), checkpoint_path)
                logging.info(f"Saved checkpoint: {checkpoint_path}")

    # Final save
    final_model_path = os.path.join(save_dir, "final_model.pt")
    torch.save(model.state_dict(), final_model_path)
    logging.info(f"Training complete. Final model saved: {final_model_path}")

    # Save tokenizer
    tokenizer.save(os.path.join(save_dir, "tokenizer"))
    logging.info("Tokenizer saved.")

# ---------- Main ----------
if __name__ == "__main__":
    CORPUS_PATH = "./corpus"
    SAVE_DIR = "./training_output"
    VOCAB_SIZE = 5000

    # 1. Load corpus
    corpus = load_corpus(CORPUS_PATH)

    # 2. Train tokenizer
    tokenizer = BPETokenizer()
    tokenizer.train(corpus, vocab_size=VOCAB_SIZE, verbose=True)

    # 3. Train model with checkpoints
    train_model(tokenizer, corpus, SAVE_DIR)
