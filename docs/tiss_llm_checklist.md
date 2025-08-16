# Tiss LLM Implementation Checklist

This document tracks the remaining engineering tasks for the QuantaTissu language model and the TissLang execution environment.

## Phase 1: TissLang Execution Engine

The TissLang parser is complete, but the execution engine that runs the parsed scripts is the next critical step. This phase focuses on building the interpreter as specified in `docs/TissLang_plan.md`.

### 1.1. Core Engine and State Management
- [ ] **`State` Class**: Implement a data class to hold execution state (`last_run_result`, `variables`, `is_halted`).
    - **Conceptualization**: A centralized, mutable container for all runtime information, enabling commands to interact and modify the execution context.
    - **Programmatic Implementations**:
        - [ ] Define `State` as a Python `dataclass` with fields for `last_run_result: Optional[Any]`, `variables: Dict[str, Any]`, and `is_halted: bool`.
        - [ ] Implement a `set_variable(name: str, value: Any)` method to safely update or add variables.
        - [ ] Implement a `reset()` method to clear variables and reset `is_halted` for new executions.
- [ ] **`ToolRegistry` Class**: Create a registry to map command types (e.g., "RUN", "WRITE") to their corresponding Python functions.
    - **Conceptualization**: A lookup mechanism that connects abstract command names from the AST to concrete Python functions that perform the actual operations.
    - **Programmatic Implementations**:
        - [ ] Use a dictionary `self._tools: Dict[str, Callable]` to store command-to-function mappings.
        - [ ] Implement `register_tool(command_type: str, func: Callable)` to add new tools to the registry.
        - [ ] Implement `get_tool(command_type: str) -> Callable` to retrieve a registered tool, raising an error if not found.
- [ ] **`ExecutionEngine` Class**: Build the main engine that iterates through the AST and dispatches commands using the tool registry.
    - **Conceptualization**: The orchestrator of the TissLang script execution, responsible for traversing the parsed program structure and invoking the correct operations.
    - **Programmatic Implementations**:
        - [ ] Initialize `ExecutionEngine` with instances of `State` and `ToolRegistry`.
        - [ ] Implement a `execute(ast: AST)` method that serves as the entry point for running a TissLang program.
        - [ ] Implement error handling within the `execute` method to catch exceptions during tool execution and update `State.is_halted`.
- [ ] **Command Dispatch Loop**: Implement the core logic in the `ExecutionEngine` to traverse the AST, including nested blocks like `STEP`.
    - **Conceptualization**: The iterative process of interpreting each node in the AST, determining its type, and executing the corresponding action or recursively processing sub-nodes.
    - **Programmatic Implementations**:
        - [ ] Use a recursive function `_traverse_node(node: ASTNode)` within `ExecutionEngine` to process the AST.
        - [ ] Implement a `match` statement or `if/elif` chain to handle different `ASTNode` types (e.g., `CommandNode`, `BlockNode`, `LiteralNode`).
        - [ ] For `CommandNode`s, extract the command type and arguments, then call `self.tool_registry.get_tool(command_type)(**args)`.

### 1.2. Command Implementation (Tooling)
- [ ] **`run_command` Tool**: Implement the logic to execute a shell command, capture its `stdout`, `stderr`, and `exit_code`, and store it in the `State` object.
    - **Conceptualization**: A bridge between the TissLang environment and the underlying operating system's shell, allowing TissLang scripts to interact with external programs.
    - **Programmatic Implementations**:
        - [ ] Use `subprocess.run(command, capture_output=True, text=True, check=False)` to execute the command.
        - [ ] Store `result.stdout`, `result.stderr`, and `result.returncode` in `State.last_run_result` as a dictionary.
        - [ ] Add a `timeout` parameter to `subprocess.run` and handle `subprocess.TimeoutExpired` exceptions.
- [ ] **`write_file` Tool**: Implement the logic to write content to a specified file path.
    - **Conceptualization**: Enables TissLang scripts to persist data or generate output files on the filesystem.
    - **Programmatic Implementations**:
        - [ ] Open the `file_path` in write mode (`'w'`) using `with open(...) as f:`.
        - [ ] Write the provided `content` string to the file (`f.write(content)`).
        - [ ] Implement `try-except` blocks to catch `IOError` or `PermissionError` during file operations.
- [ ] **`read_file` Tool**: Implement the logic to read a file's content into a variable in the `State` object.
    - **Conceptualization**: Allows TissLang scripts to ingest data or configuration from existing files on the filesystem.
    - **Programmatic Implementations**:
        - [ ] Open the `file_path` in read mode (`'r'`) using `with open(...) as f:`.
        - [ ] Read the entire file content (`f.read()`).
        - [ ] Store the read content in `State.variables[variable_name]`.
- [ ] **`assert_condition` Tool**: Implement the logic to evaluate assertion statements against the current state.
    - **Conceptualization**: Provides a mechanism for TissLang scripts to validate assumptions about the execution state, halting if a condition is not met.
    - **Programmatic Implementations**:
        - [ ] Parse the `condition` string (e.g., "x > 10") and evaluate it using `eval()` against `State.variables` (with caution due to security implications).
        - [ ] If `eval(condition)` is `False`, set `State.is_halted = True` and store an error message in `State.last_run_result`.
        - [ ] Implement a safe evaluation context for `eval()` to limit access to built-in functions and global variables.

### 1.3. Security and Sandboxing
- [ ] **Path Scoping**: Enforce strict path validation to prevent file access outside the project root.
    - **Conceptualization**: A security measure to restrict file system operations to a predefined safe area, preventing unauthorized access or modification of critical system files.
    - **Programmatic Implementations**:
        - [ ] Define a `PROJECT_ROOT` constant (e.g., `os.path.abspath(os.getcwd())`).
        - [ ] Implement `_validate_path(path: str)` utility function that checks if `os.path.abspath(path)` starts with `PROJECT_ROOT`.
        - [ ] Integrate `_validate_path` calls at the beginning of `read_file` and `write_file` tools, raising a `SecurityError` if validation fails.
- [ ] **Command Sandboxing**: Integrate a basic sandboxing mechanism for the `run_command` tool to prevent dangerous operations.
    - **Conceptualization**: A protective layer around shell command execution to mitigate risks from malicious or unintended commands, ensuring the interpreter operates within safe boundaries.
    - **Programmatic Implementations**:
        - [ ] Maintain an `ALLOWED_COMMANDS` list (e.g., `['ls', 'cat', 'echo']`) or `DENIED_KEYWORDS` (e.g., `['rm -rf', 'format']`).
        - [ ] Before executing a command in `run_command`, check if the command or its arguments match any blacklisted patterns or are not in the whitelist.
        - [ ] For more robust sandboxing, consider using `subprocess.Popen` with `preexec_fn` to drop privileges or `resource` module to limit CPU/memory (Linux/Unix only).

## Phase 2: Core Model Enhancements

This phase focuses on completing and improving the core Transformer model based on the project roadmap.

- [ ] **Causal Attention Mask**: Implement and apply the causal attention mask within the `MultiHeadAttention` layer to ensure correct autoregressive behavior during training and generation.
    - **Conceptualization**: A mechanism to prevent the model from attending to future tokens during sequence generation, preserving the autoregressive property.
    - **Programmatic Implementations**:
        - [ ] Create a `mask` tensor of shape `(seq_len, seq_len)` filled with negative infinity in the upper triangle and zeros elsewhere.
        - [ ] Add the `mask` to the attention scores (`Q @ K.T`) before applying softmax.
        - [ ] Ensure the mask is broadcastable across batch and head dimensions.
- [ ] **KV Caching**: Implement a KV Cache mechanism to accelerate generative inference by reusing key and value projections from previous tokens.
    - **Conceptualization**: A performance optimization for autoregressive decoding where previously computed key and value states are stored and reused, avoiding redundant computations.
    - **Programmatic Implementations**:
        - [ ] Modify the `MultiHeadAttention` forward pass to accept `past_key_value` tuples and return `present_key_value` tuples.
        - [ ] Concatenate `past_key` with current `key` and `past_value` with current `value` along the sequence dimension.
        - [ ] Update the model's main `forward` method to manage and pass the KV cache between decoder layers during generation.
- [ ] **CLI Interface**: Develop a command-line interface using `argparse` to allow for easy interaction with the model for generation tasks.
    - **Conceptualization**: A user-friendly command-line tool for interacting with the trained language model, enabling text generation with configurable parameters.
    - **Programmatic Implementations**:
        - [ ] Use `argparse` to define command-line arguments for `model_path`, `prompt`, `max_tokens`, `temperature`, `top_k`, etc.
        - [ ] Load the trained model and tokenizer using the provided `model_path`.
        - [ ] Implement a main function that parses arguments, initializes the model, and enters a generation loop or performs a single generation.
- [ ] **Model Checkpointing**: Add functionality to save and load model weights, which is a prerequisite for training.
    - **Conceptualization**: The ability to serialize and deserialize the model's learned parameters, allowing for training resumption, model deployment, and sharing.
    - **Programmatic Implementations**:
        - [ ] Implement `save_checkpoint(model, optimizer, epoch, loss, path)` using `torch.save({'model_state_dict': model.state_dict(), ...}, path)`.
        - [ ] Implement `load_checkpoint(path)` to load the saved state dictionaries into the model and optimizer.
        - [ ] Integrate checkpoint saving within the training loop (e.g., save every N epochs or when validation loss improves).

## Phase 3: Training Pipeline

This phase involves building the components necessary to train the model from scratch, as outlined in the roadmap.

- [ ] **Optimizer Implementation**: Implement an Adam or AdamW optimizer.
    - **Conceptualization**: An algorithm that adjusts model parameters iteratively to minimize the loss function, incorporating adaptive learning rates for efficient convergence.
    - **Programmatic Implementations**:
        - [ ] Define `AdamOptimizer` class with `__init__(params, lr, betas=(0.9, 0.999), eps=1e-8, weight_decay=0.0)` and `step()` methods.
        - [ ] Implement the Adam update rules: calculate first and second moment estimates, bias correction, and parameter update.
        - [ ] Handle `weight_decay` by adding L2 regularization to the gradients before the update.
- [ ] **Loss Function**: Integrate a cross-entropy loss function.
    - **Conceptualization**: A standard measure of the difference between two probability distributions, commonly used in classification tasks to quantify the error of a model's predictions.
    - **Programmatic Implementations**:
        - [ ] Use `torch.nn.CrossEntropyLoss()` for PyTorch, ensuring `input` (logits) and `target` (true labels) tensors are correctly shaped.
        - [ ] Apply `F.log_softmax` and `NLLLoss` manually if implementing from scratch, or use the combined `CrossEntropyLoss`.
        - [ ] Handle `ignore_index` for padding tokens if applicable.
- [ ] **Training Loop**: Create a complete training script that iterates over a dataset, performs forward and backward passes, and updates model weights.
    - **Conceptualization**: The iterative core of machine learning model training, involving data feeding, prediction, error calculation, and parameter adjustment.
    - **Programmatic Implementations**:
        - [ ] Set up data loaders for training and validation datasets.
        - [ ] Implement an outer loop for epochs and an inner loop for batches.
        - [ ] Inside the batch loop: `optimizer.zero_grad()`, `outputs = model(inputs)`, `loss = criterion(outputs, targets)`, `loss.backward()`, `optimizer.step()`.
- [ ] **Gradient Clipping**: Implement gradient clipping to prevent exploding gradients during training.
    - **Conceptualization**: A technique to constrain the magnitude of gradients during backpropagation, preventing them from becoming excessively large and destabilizing the training process.
    - **Programmatic Implementations**:
        - [ ] Call `torch.nn.utils.clip_grad_norm_(model.parameters(), max_norm=1.0)` after `loss.backward()` and before `optimizer.step()`.
        - [ ] Experiment with different `max_norm` values to find an optimal clipping threshold.
        - [ ] Monitor gradient norms during training to observe the effect of clipping.
- [ ] **Learning Rate Scheduler**: Add a learning rate scheduler to adjust the learning rate during training.
    - **Conceptualization**: A strategy to dynamically modify the optimizer's learning rate over time, often decreasing it as training progresses to fine-tune convergence.
    - **Programmatic Implementations**:
        - [ ] Choose a scheduler (e.g., `torch.optim.lr_scheduler.CosineAnnealingLR`, `ReduceLROnPlateau`).
        - [ ] Initialize the scheduler after the optimizer.
        - [ ] Call `scheduler.step()` at the end of each epoch or batch, depending on the scheduler type.

## Phase 4: Advanced Architecture & Features

This phase focuses on long-term goals to modernize the model's architecture.

- [ ] **Architectural Modernization**:
    - [ ] **RoPE (Rotary Positional Embeddings)**: Replace sinusoidal positional encodings with RoPE.
        - **Conceptualization**: A method of encoding positional information into attention mechanisms by rotating query and key vectors, allowing for better generalization to longer sequences.
        - **Programmatic Implementations**:
            - [ ] Implement a `RotaryPositionalEmbedding` module that generates rotation matrices.
            - [ ] Apply the rotation to query and key tensors within the `MultiHeadAttention` layer before computing attention scores.
            - [ ] Ensure the rotation is applied correctly for different sequence lengths and head dimensions.
    - [ ] **SwiGLU**: Replace the standard ReLU-based FFN with a SwiGLU activation.
        - **Conceptualization**: A gating mechanism within the feed-forward network that enhances model capacity and performance by introducing a multiplicative interaction between two linear transformations.
        - **Programmatic Implementations**:
            - [ ] Implement the `SwiGLU` activation function: `SwiGLU(x) = (x * sigmoid(x)) * linear_projection(x)`.
            - [ ] Replace the existing `ReLU` or `GELU` activation in the FFN layers with `SwiGLU`.
            - [ ] Adjust the hidden dimension of the FFN to accommodate the `SwiGLU` structure (typically 2/3 of the original FFN dimension).
    - [ ] **RMSNorm**: Replace `LayerNorm` with `RMSNorm` for better performance.
        - **Conceptualization**: A simplified normalization technique that scales activations by their root mean square, offering computational efficiency and improved stability in some architectures.
        - **Programmatic Implementations**:
            - [ ] Implement the `RMSNorm` module: `RMSNorm(x) = x / sqrt(mean(x^2) + epsilon)`.
            - [ ] Replace all instances of `torch.nn.LayerNorm` with the custom `RMSNorm` module in the model definition.
            - [ ] Verify that the `epsilon` value is appropriately chosen to prevent division by zero.
- [ ] **Advanced Tokenization**: Implement a subword-based tokenizer like BPE (Byte-Pair Encoding) to replace the current simple word-based tokenizer.
    - **Conceptualization**: A method to break down words into smaller, frequently occurring subword units, enabling better handling of out-of-vocabulary words and reducing vocabulary size.
    - **Programmatic Implementations**:
        - [ ] Use a library like `tokenizers` (Hugging Face) to train a BPE tokenizer on a large text corpus.
        - [ ] Implement `encode(text: str)` and `decode(token_ids: List[int])` methods for the new tokenizer.
        - [ ] Update the data loading pipeline to use the BPE tokenizer for converting text to token IDs and vice-versa.

## Phase 5: Tooling and Developer Experience

This phase focuses on improving the usability of TissLang.

- [ ] **Syntax Highlighting**: Develop a syntax highlighting extension for a major editor (e.g., VS Code).
    - **Conceptualization**: Visual cues within a code editor that differentiate various elements of the TissLang syntax, improving readability and code comprehension.
    - **Programmatic Implementations**:
        - [ ] Define a TextMate grammar file (`.tmLanguage.json`) for TissLang, specifying patterns for keywords, strings, comments, operators, etc.
        - [ ] Map these patterns to standard TextMate scopes (e.g., `keyword.control`, `string.quoted.double`).
        - [ ] Package the grammar file into a VS Code extension (`.vsix`) and provide installation instructions.
- [ ] **Linter/Static Analysis**: Create a linter to check `.tiss` files for syntax errors and potential issues before execution.
    - **Conceptualization**: An automated tool that analyzes TissLang code without executing it, identifying potential errors, style violations, and problematic patterns.
    - **Programmatic Implementations**:
        - [ ] Implement an AST visitor pattern to traverse the parsed TissLang code.
        - [ ] Define specific linting rules (e.g., check for undefined variables, unreachable code, incorrect command arguments).
        - [ ] Report issues with line numbers and descriptive error messages.
- [ ] **Dry Run Mode**: Add a `--dry-run` flag to the execution engine to simulate a run without executing commands.
    - **Conceptualization**: A simulation mode that allows users to verify the execution flow and intended actions of a TissLang script without making actual changes to the system.
    - **Programmatic Implementations**:
        - [ ] Add a `dry_run: bool` parameter to the `ExecutionEngine`'s `__init__` method.
        - [ ] Modify `run_command`, `write_file`, and other tools to check the `dry_run` flag: if `True`, log the intended action instead of performing it.
        - [ ] Provide a summary report at the end of a dry run, detailing all actions that *would* have been performed.

        