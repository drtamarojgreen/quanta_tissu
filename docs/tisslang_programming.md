# TissLang Programming Reference

This document provides a reference for **TissLang**, a high-level, declarative language designed for instructing the QuantaTissu agent. The syntax is designed to be human-readable and structured for defining agentic workflows.

---

## 1. Core Structure

A TissLang script defines a high-level task and breaks it down into sequential steps.

### `TASK`
- Defines the overall goal of the script. There should be only one `TASK` per script.
- **Syntax**: `TASK "A descriptive name for the overall goal"`

### `STEP`
- Defines a distinct, logical step within the task.
- **Syntax**: `STEP "A description of this specific step" { ... }`
- The body of the step, enclosed in curly braces `{}`, contains one or more commands.

### Example

```tiss
#TISS! Language=Python

TASK "Create and test a simple Python hello world script"

STEP "Create the main application file" {
    # Commands go here
}

STEP "Run the script and verify its output" {
    # More commands go here
}
```
- This example outlines a task to create and test a Python script, broken into two logical steps.

---

## 2. Commands

Commands are the specific actions the agent will take within a `STEP`.

### `WRITE`
- Writes a block of content to a specified file.
- **Syntax**: `WRITE "path/to/file.py" <<LANG ... LANG>>`
- The `<<LANG ... LANG>>` block is a heredoc that contains the content to be written. `LANG` is an optional language hint (e.g., `PYTHON`, `MARKDOWN`).

#### Example
```tiss
WRITE "main.py" <<PYTHON
import sys

def main():
    print(f"Hello, {sys.argv[1]}!")

if __name__ == "__main__":
    main()
PYTHON
```
- This command writes a simple Python script into the file `main.py`.

### `RUN`
- Executes a shell command and captures its output (`stdout`, `stderr`, `exit_code`).
- **Syntax**: `RUN "shell command to execute"`

#### Example
```tiss
RUN "python main.py TissLang"
```
- This command executes the `main.py` script with the argument "TissLang". The results of this run are stored in a special `LAST_RUN` context variable.

### `ASSERT`
- Checks if a condition is true and halts execution if it's false. Assertions are crucial for verifying the outcome of a `STEP`.
- **Syntax**: `ASSERT [condition]`
- Conditions operate on the state of the last `RUN` command or the file system.

#### Available Assertions:
- `LAST_RUN.STDOUT CONTAINS "some string"`
- `LAST_RUN.STDERR IS_EMPTY`
- `LAST_RUN.EXIT_CODE == 0`
- `FILE "path/to/file" EXISTS`

#### Example
```tiss
ASSERT LAST_RUN.EXIT_CODE == 0
ASSERT LAST_RUN.STDOUT CONTAINS "Hello, TissLang!"
```
- These commands verify that the previous `RUN` command executed successfully and that its standard output contained the expected greeting.

### `READ`
- Reads the content of a file into the agent's working context, assigning it to a variable.
- **Syntax**: `READ "path/to/file.py" AS my_python_code`

---

## 3. Advanced Features (Planned)

The following features are part of the TissLang roadmap and will be added in future phases.

### `IF / ELSE`
- Conditional execution of command blocks.
- **Syntax**: `IF [condition] { ... } ELSE { ... }`

### `PROMPT_AGENT`
- Explicitly invokes the underlying LLM for reasoning or generation tasks.
- **Syntax**: `PROMPT_AGENT "Natural language query" INTO [variable_name]`

### `PAUSE`
- Pauses execution and waits for human input, facilitating human-in-the-loop collaboration.
- **Syntax**: `PAUSE "Reason for pausing. Please review and type 'continue'."`

---
