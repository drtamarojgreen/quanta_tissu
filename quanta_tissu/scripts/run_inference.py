import sys
import os
import numpy as np

# Add the project root to the Python path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from quanta_tissu.model import QuantaTissu
from quanta_tissu.tokenizer import tokenize, detokenize
from quanta_tissu.config import vocab, inv_vocab

def main():
    model = QuantaTissu(vocab_size=len(vocab))
    prompt = "hello world"
    token_ids = tokenize(prompt)

    print(f"--- Prompt: '{prompt}' ---")
    print(f"Token IDs: {token_ids}\n")

    # Greedy prediction
    greedy_token = model.predict(token_ids, method="greedy")
    print(f"Greedy prediction: {detokenize([greedy_token])}")

    # Top-k sampling
    top_k_token = model.predict(token_ids, method="top_k", top_k=3, temperature=0.8)
    print(f"Top-k (k=3) prediction: {detokenize([top_k_token])}")

    # Nucleus sampling
    nucleus_token = model.predict(token_ids, method="nucleus", top_p=0.5, temperature=0.8)
    print(f"Nucleus (p=0.5) prediction: {detokenize([nucleus_token])}")

    # Random sampling with temperature
    random_token = model.predict(token_ids, method="random", temperature=1.5)
    print(f"Random (temp=1.5) prediction: {detokenize([random_token])}")

if __name__ == "__main__":
    main()
