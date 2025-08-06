# TissLang Development Plan

This document outlines the development plan for **TissLang**, a high-level, declarative language designed for instructing the QuantaTissu agent.

## 1. Overview

As the QuantaTissu agent evolves, it needs a structured, reproducible, and human-readable way to define complex software development tasks. TissLang (Tissu Language) will serve as this interface, moving beyond simple natural language prompts to a formal specification for agentic workflows.

**Core Goals:**
-   **Expressiveness**: Clearly define multi-step tasks involving file I/O, code execution, and validation.
-   **Reproducibility**: Ensure that a given TissLang script produces the same agent behavior every time.
-   **Safety**: Provide clear boundaries and assertions to prevent unintended actions.
-   **Readability**: Make agent plans easy for humans to read, review, and approve.

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
    -   `ASSERT [condition]`: Checks a condition and halts execution on failure. Conditions can include:
        -   `RUN "test.sh" SUCCEEDS`
        -   `STDOUT CONTAINS "expected output"`
        -   `FILE "path/to/file" EXISTS`

### 2.2. Parser Implementation
-   **Task**: Build a robust parser for TissLang scripts.
-   **Implementation**:
    -   Use a parser-generator library like `Lark` for Python to convert a `.tiss` script into an Abstract Syntax Tree (AST).
    -   The AST will serve as the intermediate representation for the execution engine.

### 2.3. Linter and Validator
-   **Task**: Create a static analysis tool for TissLang.
-   **Implementation**:
    -   Develop a linter that checks for syntax errors, undefined file paths, and basic logical inconsistencies before execution.

## 3. Phase 2: Agent Integration & Execution Engine

**Objective**: Connect the parsed TissLang script to the QuantaTissu agent's tool-use capabilities.

### 3.1. Execution Engine (AST Walker)
-   **Task**: Build an interpreter that executes the logic defined in the AST.
-   **Implementation**:
    -   The engine will traverse the AST node by node.
    -   For each action node (e.g., `RUN`, `WRITE`), it will invoke the corresponding agent tool (`run_shell`, `write_file`).

### 3.2. State and Context Management
-   **Task**: Manage the state of the execution environment.
-   **Implementation**:
    -   The engine will track the output (stdout, stderr, exit code) of the last `RUN` command.
-   It will manage a list of files currently held in the agent's working context and any user-defined variables.

### 3.3. Assertion and Correction Loop
-   **Task**: Implement the `ASSERT` logic to validate steps and trigger corrections.
-   **Implementation**:
    -   The engine will evaluate `ASSERT` conditions against the current state.
    -   On a failed assertion, it will halt and feed the failure context back into the agent's self-correction module, prompting it for a fix.

## 4. Phase 3: Advanced Features & Self-Generation

**Objective**: Enhance TissLang with more powerful programming constructs and enable the agent to generate its own TissLang plans.

### 4.1. Advanced Language Features
-   **Task**: Add variables, control flow, and modularity.
-   **Implementation**:
    -   **Variables**: `VAR output = LAST_RUN.stdout`.
    -   **Control Flow**: `IF [condition] { ... } ELSE { ... }`.
    -   **Sub-tasks**: `DEFINE_TASK "my_sub_task" { ... }` to create reusable procedures.
    -   **Error Handling**: `TRY { ... } CATCH { ... }` to gracefully handle failing commands.

### 4.2. Agent-Driven Plan Generation
-   **Task**: Teach the QuantaTissu model to write TissLang.
-   **Implementation**:
    -   Fine-tune the model on a dataset of `(natural_language_goal, tisslang_script)` pairs.
    -   The agent's first step when given a high-level goal will be to generate a `.tiss` script as its plan of action, which a human can then review and approve.

## 5. Definition of Done
-   A `.tiss` script can be written, linted, and successfully parsed into an AST.
-   The agent's execution engine can run a simple script involving file I/O, shell commands, and assertions.
-   The agent can generate a valid `.tiss` script as a plan in response to a natural language prompt.

## 6. Phase 4: Ecological Awareness & Cost Analysis

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

## 7. Phase 5: Human-in-the-Loop Collaboration

**Objective**: Enhance TissLang with commands that facilitate seamless and explicit collaboration between the agent and a human developer.

### 7.1. Interactive Commands
-   **Task**: Add keywords that pause execution and require human interaction.
-   **Implementation**:
    -   `PAUSE "Reason for pausing. Please review the changes in file.py and type 'continue'."`: Explicitly stop execution and wait for human input.
    -   `REQUEST_REVIEW "Does this implementation of `my_func` look correct and efficient?"`: A formal request for a human code review.
    -   `CHOOSE { OPTION "Use a fast but memory-intensive algorithm" { ... } OPTION "Use a slower, memory-efficient algorithm" { ... } }`: Present multiple implementation strategies to the human and wait for their choice before proceeding.