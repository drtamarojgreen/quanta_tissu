# TissLM v0.0.0: High-Level Plan

This document outlines a high-level plan to address the limitations identified in TissLM v0.0.0.

## Phase 1: Stabilize the Core

The first priority is to fix the critical and major issues to create a stable foundation.

1.  **Fix Tokenizer Encoding/Decoding:**
    *   **Task:** Overhaul the tokenizer's handling of UTF-8 characters. This will likely involve moving away from simple byte-level manipulation and using a proper UTF-8 library.
    *   **Goal:** The `encoding_test` should pass, and the model should be able to correctly handle a wide range of text, including special characters, emojis, and non-latin scripts.

2.  **Fix KV Cache Correctness:**
    *   **Task:** Debug the KV cache implementation to identify the source of the output mismatch. This will require careful comparison of the cached and non-cached generation paths.
    *   **Goal:** The `kv_cache_test` should pass with `Correctness: ✓`, and the cache should be a reliable performance optimization.

3.  **Stabilize Database Integration:**
    *   **Task:** Investigate the `JOIN` query failure in `test_db_and_retrieval` and the HTTP 500 error in `tissdb_lite_integration_test`. This may require debugging the database itself or the client-side code.
    *   **Goal:** All database-related tests should pass, and the integration with TissDB should be reliable.

## Phase 2: Improve Generation Quality

Once the core is stable, the focus will shift to improving the quality of the generated text.

1.  **Train a Better Tokenizer:**
    *   **Task:** Train a new BPE tokenizer on a much larger and more diverse corpus.
    *   **Goal:** The tokenizer should have a larger vocabulary that is more representative of the target domain.

2.  **Train the Model:**
    *   **Task:** Train the TissLM model on a large corpus for a sufficient number of epochs.
    *   **Goal:** The model should be able to generate coherent and contextually relevant text.

3.  **Implement and Tune RAG:**
    *   **Task:** Implement a functional RAG pipeline. This will involve integrating the retrieval module with the generator and tuning the retrieval and generation components.
    *   **Goal:** The `rag_self_update_test` should pass, and the model should be able to use retrieved documents to generate more informative and accurate responses.

## Phase 3: Enhance and Expand

With a stable and functional model, we can focus on adding new features and enhancements.

1.  **Implement Advanced Sampling Methods:**
    *   **Task:** Implement and test the advanced sampling methods that are currently placeholders (e.g., contrastive search, Mirostat).
    *   **Goal:** Provide users with a wider range of options for controlling the text generation process.

2.  **Improve Rule Enforcement:**
    *   **Task:** Enhance the rule enforcement capabilities to allow for more complex and fine-grained control over the generated text.
    *   **Goal:** Enable users to specify and enforce a variety of stylistic and grammatical rules.

3.  **Expand the API:**
    *   **Task:** Expose more of the model's functionality through the C++ and Python APIs.
    *   **Goal:** Give users more control over the model and its components.
