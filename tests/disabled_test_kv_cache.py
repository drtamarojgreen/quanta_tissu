import sys
import os
import numpy as np
import unittest

# Add the project root to the Python path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from quanta_tissu.tisslm.model import QuantaTissu
from quanta_tissu.tisslm.config import model_config
from quanta_tissu.tisslm.tokenizer import Tokenizer

class TestKVCache(unittest.TestCase):

    def setUp(self):
        """Set up a model and tokenizer for the tests."""
        np.random.seed(42)
        self.model = QuantaTissu(model_config)
        self.tokenizer = Tokenizer()

    def test_kv_cache_correctness(self):
        """
        Tests that generating with KV cache produces the exact same output
        as generating without it.
        """
        prompt = "hello"
        n_new_tokens = 5

        # --- 1. Generate with the new, efficient `generate` method (with KV cache) ---
        prompt_tokens = self.tokenizer.tokenize(prompt)
        cached_generated_ids = self.model.generate(
            prompt_tokens,
            n_new_tokens=n_new_tokens,
            method="greedy"
        )

        # --- 2. Generate with the old, inefficient `predict` method (without cache) ---
        # We manually recreate the generation loop here for a baseline comparison.
        non_cached_generated_ids = []
        current_tokens = list(prompt_tokens)
        for _ in range(n_new_tokens):
            token_ids_np = np.array(current_tokens)
            next_token_id = self.model.predict(token_ids_np, method="greedy")
            non_cached_generated_ids.append(next_token_id)
            current_tokens.append(next_token_id)

        # --- 3. Compare the results ---
        self.assertEqual(
            cached_generated_ids,
            non_cached_generated_ids,
            "Generation with KV cache should produce the same tokens as without it."
        )
        print("\nKV Cache Correctness Test Passed!")
        print(f"Prompt: '{prompt}'")
        print(f"Generated tokens (cached):   {cached_generated_ids}")
        print(f"Generated tokens (no cache): {non_cached_generated_ids}")


if __name__ == '__main__':
    unittest.main()
