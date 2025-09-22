# TissLM Evaluation Guide

This document outlines the current status of the TissLM (Tissu Language Model) scripts, the available evaluation tools, and the strategic direction for future development.

---

## 1. Overview

TissLM is a from-scratch, transformer-based language model implemented in Python using NumPy. As detailed in the main `README.md`, it was built for educational purposes and has evolved into a capable system with a full training and inference pipeline.

The project currently consists of two primary sets of scripts:
1.  **`quanta_tissu/tisslm/legacylm/`**: The core, from-scratch training and inference system.
2.  **`quanta_tissu/tisslm/evaluation/`**: A comprehensive but decentralized suite of tools for evaluating the model's performance on various tasks.

---

## 2. Core Scripts Status (`legacylm`)

The `legacylm` directory contains the foundational scripts for the NumPy-based model.

### `train.py`
*   **Purpose**: This is the main script for training the TissLM from a text corpus.
*   **Current Status**: **Functional**.
*   **Features**:
    *   **From-Scratch Implementation**: Implements the full training pipeline, including a custom `AdamOptimizer` and a learning rate scheduler.
    *   **Data Handling**: Loads a corpus from a text file and uses a `TissTokenizer` for tokenization.
    *   **Training Loop**: Executes a standard training loop involving forward pass, cross-entropy loss calculation, and backpropagation.
    *   **Checkpointing**: Periodically saves model weights and optimizer state to `.npz` files.
*   **To Be Implemented / Challenges**:
    *   **No Integrated Validation**: The script lacks a validation loop to monitor for overfitting and save the best model based on a validation metric. Evaluation is a manual, post-training process.
    *   **Performance**: Being pure NumPy, it is CPU-bound and significantly slower than models built on frameworks like PyTorch or TensorFlow, limiting the practical scale of training.

### `run_inference.py`
*   **Purpose**: Generates text using a trained TissLM checkpoint.
*   **Current Status**: **Functional**.
*   **Features**:
    *   Supports multiple decoding strategies, including greedy, top-k, and nucleus (top-p) sampling.
    *   Loads a model checkpoint and tokenizer to produce output from a given prompt.

---

## 3. Evaluation Scripts Status (`evaluation/`)

The `evaluation` directory is a workbench of scripts for model analysis, data generation, and performance measurement. It is highly functional but not unified into a single framework.

### `evaluation_metrics.py`
*   **Purpose**: A library defining the core metrics used across evaluation scripts.
*   **Current Status**: **Implemented**.
*   **Available Metrics**:
    *   `calculate_perplexity`: For measuring how well the model predicts a sample of text.
    *   `calculate_bleu`: For comparing generated text against a reference (e.g., for summarization).
    *   `calculate_rouge`: (ROUGE-1, ROUGE-2, ROUGE-L) Also for summarization evaluation.
    *   `calculate_code_similarity`: Uses `difflib` to measure the textual similarity between generated and reference code.
    *   `functional_correctness`: A powerful metric that executes generated code in a secure subprocess and verifies its correctness against a test case.

### Key Evaluation Scripts
*   **`evaluate_corpus.py`**:
    *   **Purpose**: Calculates the perplexity of a trained model over an arbitrary corpus.
    *   **Status**: **Functional**. It serves as the primary tool for general-purpose language modeling evaluation.

*   **`run_code_generation.py`**:
    *   **Purpose**: A task-specific script to evaluate the model's Python code generation capabilities.
    *   **Status**: **Functional**. It uses `calculate_code_similarity` and `functional_correctness` to provide a robust measure of the model's coding ability.

*   **`run_text_summarization.py`**:
    *   **Purpose**: A task-specific script to evaluate text summarization.
    *   **Status**: **Functional**. It likely uses the BLEU and ROUGE metrics.

*   **Other Scripts**: The directory contains numerous other scripts for debugging (`training_debugger.py`), model inspection (`model_inspector.py`), and data generation (`generate_algorithmic_suite.py`), reflecting a history of experimentation.

---

## 4. Summary of Progress & Challenges

### Progress
*   A complete, from-scratch training and inference pipeline for a transformer model has been successfully implemented.
*   A rich and diverse suite of evaluation metrics and scripts exists, covering general perplexity, summarization, and advanced code generation correctness.
*   The project has demonstrated the ability to evaluate complex, task-specific capabilities like functional code generation.

### Current Challenges & What's Left
1.  **Decentralized Evaluation**: The primary challenge is the lack of a unified evaluation harness. Running evaluations requires executing multiple, separate scripts with different arguments, making systematic testing difficult.
2.  **Scalability**: The NumPy backend is a major bottleneck for scaling the model and accelerating research and development.
3.  **No Standard Benchmarks**: The model has not been evaluated against well-known academic benchmarks (e.g., GLUE, SuperGLUE, HumanEval), making it difficult to compare its performance to other models.
4.  **Documentation**: Many of the specific evaluation scripts lack dedicated documentation for their parameters and expected usage.

---

## 5. Future Implementation Roadmap

1.  **Develop a Unified Evaluation CLI**:
    *   Create a single entry-point script (e.g., `python -m evaluation.main --task all --model_path ...`) that can run a configurable suite of evaluations (perplexity, summarization, code-gen) and aggregate the results into a single report.

2.  **Integrate Standard Benchmarks**:
    *   Add support for downloading and evaluating on standard datasets like those from the GLUE benchmark for NLU tasks and HumanEval for code generation.

3.  **Port Model to a Modern Framework**:
    *   To overcome performance limitations, the model architecture should be ported to PyTorch, JAX, or TensorFlow. This would enable GPU acceleration and unlock the ability to train larger, more powerful models.

4.  **Integrate Validation into Training**:
    *   The `train.py` script should be updated to include a validation loop that runs periodically on a held-out dataset, allowing for early stopping and saving the best-performing model checkpoint.
    
    
    # TissLM IDE Planning Phases

This document summarizes the discussions and planning for the TissLM IDE features, including their current status.

## 1. Core IDE Features Under Discussion

**Initial Scope:** Debugging, Refactoring, Search & Replace, Testing Integration. (Navigation was initially discussed but later removed from scope).

### 1.1. Debugging

**Description:**
*   **Breakpoints:** Set points in the code where execution will pause, allowing inspection of the program's state.
*   **Step-Through Execution:**
    *   **Step Over:** Execute the current line of code and move to the next, stepping over function calls.
    *   **Step Into:** Execute the current line and, if it's a function call, step into that function's code.
    *   **Step Out:** Execute the remainder of the current function and return to the calling line.
*   **Variable Inspection:** View the current values of variables in scope at a breakpoint.
*   **Call Stack:** Display the sequence of function calls that led to the current execution point.

**Status:** Completed (Documentation)

### 1.2. Refactoring

**Description:**
*   **Rename:** Safely change the name of a symbol (e.g., variable, function, class) across all occurrences in the project, ensuring all references are updated.
*   **Extract Method/Function:** Select a block of code and automatically move it into a new method or function, replacing the original block with a call to the new method.
*   **Extract Variable:** Select an expression and replace it with a new variable, assigning the expression's value to the variable.

**Status:** Completed (Documentation)

### 1.3. Search & Replace

**Description:**
*   **Text Search:** Find specific text strings within the current file or across the entire project.
*   **Regular Expression Search:** Use regular expressions for more powerful and flexible text pattern matching.
*   **Search in Selection:** Limit the search operation to a highlighted block of text.
*   **Replace All:** Replace all occurrences of a found text string with a new string, either in the current file or across the project.

**Status:** Completed (Documentation)

#### 1.3.1. Command-Line Interface (CLI) Proposal for Search & Replace

**Common Usage Pattern:**
```bash
<ide_command> search-replace [options]
```
*   `<ide_command>` would be `tiss_cpp_ide` for the C++ IDE and `tiss_python_ide` for the Python IDE.

**Options:**
*   **`--pattern <string>` (Required):** The text string or regular expression pattern to search for.
*   **`--replace <string>` (Optional):** The string to replace the found `--pattern` with. If omitted, only search is performed.
*   **`--files <glob_pattern>` (Optional):** A glob pattern to filter which files to include in the search (e.g., `src/**/*.cpp`, `tests/*.py`). Defaults to all relevant source files.
*   **`--regex` (Flag, Optional):** Treats `--pattern` as a regular expression.
*   **`--case-sensitive` (Flag, Optional):** Performs a case-sensitive search.
*   **`--in-place` (Flag, Optional):** If used with `--replace`, modifies files directly. **Caution advised.**
*   **`--dry-run` (Flag, Optional):** If used with `--replace`, shows a preview of changes without modifying files.
*   **`--output <file_path>` (Optional):** Writes search results to a specified file.

**Status:** Proposed (CLI Specification)

### 1.4. Testing Integration

**Description:**
*   **Run Tests:** Initiate the execution of defined tests within the IDE.
*   **View Test Results:** Display the outcome of test runs, indicating which tests passed, failed, or were skipped, including error messages or stack traces for failed tests.

**Status:** Completed (Documentation)

## 2. IDE Locations

The IDEs are located at:
*   Python IDE: `quanta_tissu/ide/p`
*   C++ IDE: `quanta_tissu/ide/c`

**Status:** Noted

## 3. Implementation Capabilities

As an AI, I can assist in implementing these features iteratively by writing code snippets, modifying existing files, and helping with debugging. I cannot autonomously implement complex features from scratch in a single step.

**Status:** Clarified

---

# Understanding the Tokenizer Issue

## 1. Initial Problem Description

When attempting to generate text using `quanta_tissu.tisslm.core.generate_text` (and observed in `evaluation.generate_algorithmic`), the output was consistently garbled and highly repetitive (e.g., "amino amino amino..."). The generated text was appended to the prompt, but the generated portion was nonsensical.

## 2. Initial Hypotheses

Two primary hypotheses were considered for the garbled output:
*   **Undertrained Model:** The language model (`QuantaTissu`) might not have been sufficiently trained, leading to random or repetitive token generation.
*   **Tokenizer Issues:** The `Tokenizer` class, specifically its `tokenize` or `detokenize` methods, might be faulty, leading to incorrect conversion between text and token IDs.

## 3. Tokenizer Test and Observation

To isolate the problem, a dedicated test script (`quanta_tissu/tisslm/evaluation/test_tokenizer.py`) was created to evaluate the `Tokenizer`'s `tokenize` and `detokenize` methods in isolation using a simple phrase, "hello world".

**Observed Output:**
```
Original phrase: hello world
Token IDs: [259, 108, 653, 539]
Detokenized text: he l lo  world
```

This output clearly demonstrated that the `Tokenizer` was unable to correctly reconstruct the original phrase. The `detokenized_text` was fragmented and incorrectly spaced, confirming a direct problem with the tokenizer's implementation.

## 4. Analysis of `tokenizer.py` (`detokenize` method)

Upon reviewing the `detokenize` method in `quanta_tissu/tisslm/core/tokenizer.py`, the following observations were made:

*   The method attempts to re-insert spaces based on a heuristic: it adds a space before each token unless it's the first token or a punctuation mark.
*   This heuristic-based space re-insertion is a common source of errors in BPE (Byte Pair Encoding) detokenization.
*   Typically, BPE tokenizers embed space information directly within the tokens themselves (e.g., by prefixing words with a special character or including spaces as part of the token). If the underlying `self.bpe_tokenizer.decode` method already handles this, then the manual space re-insertion becomes redundant and incorrect, leading to fragmented output like the observed "he l lo world".

## 5. Proposed Solution (Conceptual)

The most straightforward solution is to simplify the `detokenize` method. If the `self.bpe_tokenizer.decode` method correctly handles space information (which is standard for BPE implementations), then simply joining the `decoded_tokens` should be sufficient.

**Proposed Change (Conceptual):**
Replace the complex space re-insertion logic with a direct join:
```python
        text = "".join(decoded_tokens)
```

## 6. Implementation Attempt and Challenge

An attempt was made to apply this change using the `replace` tool. However, the operation failed due to a precise string matching issue, likely related to subtle whitespace, indentation, or hidden characters within the multi-line `old_string` argument. This indicates that direct programmatic replacement of the large block is challenging with the current tool capabilities.

## 7. Next Steps

Proceed with manual implementation of the proposed solution for the `detokenize` method in `quanta_tissu/tisslm/core/tokenizer.py`.
