# TissLM Program Implementation Notes

This document lists placeholders, todos, and other implementation details for the TissLM C++ program.

## 1. `Generator::beam_search`

**Issue:** The current implementation of `beam_search` is inefficient.

**Details:** The method re-processes the entire sequence for each beam in every step. A proper implementation should use a KV cache for each beam to avoid redundant computations.

**TODO:** Refactor `beam_search` to manage a KV cache for each beam.

## 2. `Generator::speculative_sampling`

**Issue:** The `speculative_sampling` method is a placeholder.

**Details:** The current implementation defaults to greedy sampling. It does not use a draft model to speed up inference.

**TODO:** Implement speculative sampling with a draft model.

## 3. `Tokenizer::decode`

**Issue:** The `decode` method has a simplified UTF-8 handling.

**Details:** The current implementation might not be as robust as the Python version for handling all edge cases of invalid UTF-8 sequences.

**TODO:** Improve the robustness of the UTF-8 decoding in the `decode` method.

## 4. `contrastive_search` KV cache

**Issue:** The `contrastive_search` method is inefficient.

**Details:** Similar to `beam_search`, the method re-processes the entire sequence in every step.

**TODO:** Refactor `contrastive_search` to use a KV cache.

## 5. `mirostat_sampling` KV cache

**Issue:** The `mirostat_sampling` method is inefficient.

**Details:** Similar to `beam_search`, the method re-processes the entire sequence in every step.

**TODO:** Refactor `mirostat_sampling` to use a KV cache.
