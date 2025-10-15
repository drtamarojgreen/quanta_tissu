# TissLM Program Progress Report

This document summarizes the progress made in implementing the TissLM C++ program, the remaining differences with the Python prototype, and a strategy for configuration management.

## Accomplishments

1.  **Database Tests Refactoring:**
    -   The C++ database tests in `test_db_and_retrieval.cpp` have been refactored to use TissQL queries, aligning them with the Python BDD tests.
    -   A `query` method was added to `TissDBClient` to send TissQL queries to the `/_query` endpoint.
    -   The tests for CRUD operations, feedback, multiple documents, and search have been updated to use TissQL.
    -   Database cleanup is now performed at the beginning of the test run to prevent "database already exists" errors.
    -   New tests for `JOIN` and `GROUP BY` have been added to identify more database issues.

2.  **C++ Tokenizer Implementation:**
    -   The C++ `Tokenizer` has been significantly improved to match the functionality of the Python `BPETokenizer`.
    -   A `train` method has been implemented to train a new tokenizer from a text corpus.
    -   A `save` method has been implemented to save the trained tokenizer.
    -   The `encode` method has been made more efficient.
    -   The `decode` method has been made more robust with proper UTF-8 handling.
    -   A new test `test_tokenizer_training` has been added to verify the new functionality.

3.  **C++ Generator Extension:**
    -   The C++ `Generator` has been extended with several new sampling methods and features from the Python prototype:
        -   `random` sampling
        -   Repetition penalty
        -   Logit bias
        -   N-gram repetition penalty
        -   `beam_search` (placeholder implementation)
        -   `contrastive_search` (placeholder implementation)
        -   `mirostat_sampling` (placeholder implementation)
        -   `speculative_sampling` (placeholder implementation)
    -   New tests have been added to `standard_generation_test.cpp` for each of the new features.

4.  **Documentation:**
    -   `docs/tissdb_necessary_upgrades.md`: Created to document server-side bugs found during testing.
    -   `docs/tisslm_program_implementation.md`: Created to document placeholders and todos in the C++ implementation.

## Remaining Differences (Python vs. C++)

### Tokenizer

-   **`bpe_encode` efficiency:** The C++ `bpe_encode` is still less efficient than the Python version. The Python version finds the best pair to merge based on the learned merge order, while the C++ version iterates through all pairs.

### Generator

-   **Advanced Sampling Methods:** The C++ implementations of `beam_search`, `contrastive_search`, `mirostat_sampling`, and `speculative_sampling` are placeholders and need to be fully implemented.
-   **KV Cache:** The advanced sampling methods in C++ do not yet use a KV cache, which is crucial for efficiency.
-   **RAG / Context:** The `sample_with_context` method for Retrieval-Augmented Generation is missing in the C++ generator.

### Training Pipeline

-   **Data Loading:** The C++ training pipeline uses a simple `TokenDataset` and manual batching. The Python version uses `torch.utils.data.DataLoader`, which is more advanced and efficient.
-   **Optimizer:** The C++ version has an `Adam` optimizer, but the Python version might have more options.
-   **Evaluation:** An evaluation pipeline similar to `complete_evaluations.py` is missing in C++.

## Configuration Strategy

To avoid hardcoding parameters and to make the C++ programs more flexible, a configuration file system should be implemented.

**Format:**
A simple key-value text file format will be used. Each line will contain a key-value pair, separated by an equals sign (`=`). Comments can be added using a hash symbol (`#`).

**Proposed Configuration Files:**

1.  **`quanta_tissu/tisslm/program/config/model.conf`:** For the core model parameters.
    ```
    # Model Configuration
    vocab_size = 5000
    seq_len = 128
    embed_dim = 128
    num_heads = 4
    num_layers = 2
    dropout_rate = 0.1
    ```

2.  **`quanta_tissu/tisslm/program/pipelines/config/training.conf`:** For the training pipeline.
    ```
    # Training Configuration
    corpus_path = ../../corpus/corpus.txt
    save_dir = ../../training_output
    batch_size = 32
    epochs = 5
    learning_rate = 0.001
    ```

3.  **`tests/model/program/config/test.conf`:** For the tests.
    ```
    # Test Configuration
    model_path = ../../../training_output/final_model.pt
    tokenizer_prefix = ../../../training_output/tokenizer
    prompt = The meaning of life is
    generation_length = 100
    ```

**Implementation:**

1.  **`Config` Class:**
    A `Config` class will be created in `quanta_tissu/tisslm/program/core/` to load and parse the key-value configuration files.

    `config.h`:
    ```cpp
    #pragma once

    #include <string>
    #include <map>

    namespace TissDB {
    namespace TissLM {
    namespace Core {

    class Config {
    public:
        Config(const std::string& config_path);

        std::string get_string(const std::string& key, const std::string& default_value = "") const;
        int get_int(const std::string& key, int default_value = 0) const;
        float get_float(const std::string& key, float default_value = 0.0f) const;

    private:
        std::map<std::string, std::string> params_;
    };

    } // namespace Core
    } // namespace TissLM
    } // namespace TissDB
    ```

2.  **Integration into C++ Programs:**
    Each C++ program (`run_training`, `standard_generation_test`, etc.) will be modified to:
    -   Load its specific configuration file from a fixed relative path.
    -   Create a `Config` object.
    -   Use the `Config` object to retrieve all necessary parameters.
