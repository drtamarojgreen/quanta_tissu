# TissLM v0.0.0: Capabilities and Limitations

This document outlines the current state of the TissLM model as of version 0.0.0.

## Capabilities

The model has a solid foundation with many core features implemented and passing basic tests.

*   **Core Model Architecture:** The transformer model, including embedding, positional encoding, attention, and feed-forward layers, is functional.
*   **Tokenizer:** A Byte-Pair Encoding (BPE) tokenizer can be trained from a corpus, and can encode and decode basic ASCII text.
*   **Training:** The model can be trained using an Adam optimizer and Cross-Entropy loss function.
*   **Text Generation:** A variety of text generation methods are implemented, including:
    *   Greedy search
    *   Top-k sampling
    *   Nucleus sampling
    *   Experimental methods like beam search, contrastive search, and Mirostat sampling.
*   **KV Cache:** A Key-Value cache for faster inference is implemented and provides a significant speedup.
*   **Rule Enforcement:** Basic rule enforcement for generated text, such as repetition penalty, is in place.
*   **Advanced Analysis:** The model has capabilities for advanced text analysis, such as bigram frequency.
*   **Checkpointing:** The model's state can be saved to and loaded from checkpoints.

## Limitations

Despite the capabilities, there are several critical and major limitations that need to be addressed.

*   **Critical: Tokenizer Encoding/Decoding:** The tokenizer fails to correctly handle special characters, emojis, and non-latin scripts. This is the most critical issue, as it leads to incorrect text generation and the appearance of replacement characters (``) in the output.
*   **Critical: KV Cache Correctness:** The KV cache implementation has a correctness bug. While it provides a speedup, it produces different results compared to generation without the cache. This makes it unreliable for production use.
*   **Major: Database Integration:** The integration with TissDB is not stable. The `test_db_and_retrieval` test fails on a `JOIN` query, and the `tissdb_lite_integration_test` fails completely with an HTTP 500 error.
*   **Major: RAG Pipeline:** The Retrieval-Augmented Generation (RAG) pipeline is not functional. The tests for RAG scenarios are failing.
*   **Low-Quality Generation:** The quality of the generated text is very low. This is expected from an untrained model with a small vocabulary, but it is a major limitation for any practical application.
