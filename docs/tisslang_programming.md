# TissLang Programming Reference

This document provides a comprehensive reference for **TissLang**, a high-level, declarative language designed for instructing the QuantaTissu agent. The syntax is designed to be human-readable and structured for defining agentic workflows.

---

## 1. Core Concepts

A TissLang script defines a high-level task and breaks it down into sequential steps.

### `TASK`
The `TASK` declaration is the root of a TissLang script. It defines the overall goal that the agent should accomplish. There must be exactly one `TASK` per script.

- **Syntax**: `TASK "A descriptive name for the overall goal"`

### `STEP`
A `STEP` defines a distinct, logical unit of work within the task. Steps are executed sequentially by default.

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

---

## 2. Commands

Commands are the specific actions the agent will take within a `STEP`.

### `WRITE`
Writes a block of content to a specified file. This is useful for creating source code, configuration files, or any text-based artifact.

- **Syntax**: `WRITE "path/to/file.py" <<LANG ... LANG>>`
- The `<<LANG ... LANG>>` block is a heredoc that contains the content to be written. `LANG` is a language hint (e.g., `PYTHON`, `MARKDOWN`).

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

### `RUN`
Executes a shell command and captures its output (`stdout`, `stderr`, `exit_code`). The results of the command are stored in a special `LAST_RUN` context variable that can be used for assertions.

- **Syntax**: `RUN "shell command to execute"`

#### Example
```tiss
RUN "python main.py TissLang"
```

### `ASSERT`
Checks if a condition is true and halts execution if it's false. Assertions are crucial for verifying the outcome of a `STEP` and ensuring the workflow is on track.

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

### `READ`
Reads the content of a file into the agent's working context, assigning it to a variable for later use, typically with `PROMPT_AGENT`.

- **Syntax**: `READ "path/to/file.py" AS my_python_code`

### `PROMPT_AGENT`
Instructs the agent to use its underlying language model to reason, generate, or refactor content. This is the primary way to leverage the AI's capabilities.

- **Syntax**: `PROMPT_AGENT "Natural language query" [INTO variable_name]`
- The `INTO` clause is optional. If present, the agent's response is stored in the specified variable for later use.
- You can insert the content of variables into the prompt using `{{variable_name}}` syntax.

#### Example
```tiss
STEP "Read the original file" {
    READ "./project/utils.py" AS original_code
}

STEP "Prompt agent to refactor for clarity" {
    PROMPT_AGENT "Refactor this Python code for better readability and add docstrings: {{original_code}}" INTO refactored_code
}
```

### `SET_BUDGET`
Sets a resource constraint for the current task. This is part of TissLang's ecological awareness features, allowing developers to control the computational resources an agent can consume.

- **Syntax**: `SET_BUDGET [resource_type] = [value]`
- Common resource types include `EXECUTION_TIME` and `API_CALLS`.

#### Example
```tiss
TASK "Analyze user activity from the past 24 hours" {
    SET_BUDGET EXECUTION_TIME = "5m"
    SET_BUDGET API_CALLS = 5

    # ... subsequent steps
}
```

### `REQUEST_REVIEW`
Pauses execution and asks for human input, providing a message to the user. This is essential for tasks that require human oversight or decision-making. It is a more structured alternative to `PAUSE`.

- **Syntax**: `REQUEST_REVIEW "Message to the human reviewer"`

#### Example
```tiss
REQUEST_REVIEW "Analysis took over 5 minutes. Should I proceed with generating the summary?"
```

---

## 3. Control Flow

TissLang provides constructs to manage the flow of execution, allowing for parallel operations and conditional logic.

### `PARALLEL`
The `PARALLEL` block allows you to define multiple `STEP`s that the agent can execute concurrently. This is useful for tasks that can be broken down into independent sub-problems, such as testing different approaches or generating multiple pieces of content at once.

- **Syntax**: `PARALLEL { ... }`

#### Example: A/B Testing Prompts
```tiss
TASK "Find the best prompt to summarize a complex article" {
    READ "./articles/quantum_mechanics.txt" AS article_text

    PARALLEL {
        STEP "Test Prompt A (Simple)" {
            PROMPT_AGENT "Summarize this article: {{article_text}}" INTO summary_a
        }
        STEP "Test Prompt B (Role-based)" {
            @persona "science_journalist"
            PROMPT_AGENT "Summarize this article for a general audience: {{article_text}}" INTO summary_b
        }
    }

    STEP "Evaluate which summary is better" {
        # ... logic to compare summary_a and summary_b
    }
}
```

### `CHOOSE`
The `CHOOSE` block presents the agent with multiple, mutually exclusive `STEP`s. The agent must decide which single `STEP` to execute based on its reasoning, directives, or prior results. This is powerful for creating adaptable, decision-making workflows.

- **Syntax**: `CHOOSE { ... }`

#### Example: Green-Aware Planning
```tiss
TASK "Find the most computationally efficient way to sort a large dataset" {
  CHOOSE {
    STEP "Option A: Quicksort" {
      @persona "performance_engineer"
      ESTIMATE_COST {
        PROMPT_AGENT "Implement a quicksort algorithm in Python."
      }
    }
    STEP "Option B: Mergesort" {
      @persona "systems_engineer"
      ESTIMATE_COST {
        PROMPT_AGENT "Implement a memory-efficient mergesort algorithm in Python."
      }
    }
  }
}
```

### `ESTIMATE_COST`
The `ESTIMATE_COST` block is a special construct used within a `CHOOSE` block. It instructs the agent to evaluate the potential resource cost of the commands within it without actually executing them. The agent then uses these estimates to inform its choice.

- **Syntax**: `ESTIMATE_COST { ... }`

---

## 4. Directives

Directives are special instructions, prefixed with an `@` symbol, that control the agent's runtime behavior or set metadata for a specific block. They can be applied at the `TASK` or `STEP` level.

- **Syntax**: `@directive_name value`

### Common Directives

- **`@persona`**: Instructs the agent to adopt a specific persona or role when executing the commands within the block (e.g., `PROMPT_AGENT`). This can influence the tone, style, and content of the agent's output.
- **`@output`**: Specifies the desired format for artifacts generated within the block (e.g., `json`, `xml`).
- **`@language`**: Provides a hint to the TissLang interpreter about the primary programming language being used in the script (e.g., `#TISS! Language=Python`).

#### Example: Using Directives
```tiss
TASK "Explain quantum computing and save as JSON" {
    @output "json"

    STEP "Explain with rigor" {
        @persona "physicist"
        PROMPT_AGENT "Explain quantum superposition and entanglement." INTO rigorous_explanation
    }

    STEP "Explain with analogy" {
        @persona "teacher"
        PROMPT_AGENT "Create a simple analogy for quantum superposition." INTO simple_analogy
    }

    STEP "Synthesize final answer" {
        @persona "editor"
        PROMPT_AGENT "Combine the rigorous explanation ({{rigorous_explanation}}) with the simple analogy ({{simple_analogy}})." INTO final_explanation
    }
}
```
In this example:
- `@output "json"` at the `TASK` level might instruct the agent to save the final output as a JSON object.
- `@persona "physicist"` and `@persona "teacher"` guide the agent to generate content from different perspectives in parallel steps.
- `@persona "editor"` instructs the agent to synthesize the previous outputs into a cohesive whole.

---

## 5. Advanced Examples

This section showcases complete TissLang scripts that solve complex, multi-step problems by combining multiple commands, control flow blocks, and directives.

### Example 1: Self-Correcting Code Refactoring
This pipeline demonstrates a common agentic loop: read, modify, write, and test. A real-world interpreter could be configured to loop back to the "refactor" step if the final assertion fails.

```tiss
TASK "Refactor the 'utils.py' module and ensure tests pass" {
    STEP "Read the original file" {
        READ "./project/utils.py" AS original_code
    }

    STEP "Prompt agent to refactor for clarity" {
        @persona "senior_developer"
        PROMPT_AGENT "Refactor this Python code for better readability and add docstrings: {{original_code}}" INTO refactored_code
    }

    STEP "Write the new code to the file" {
        WRITE "./project/utils.py" <<PYTHON
{{refactored_code}}
PYTHON
    }

    STEP "Run unit tests" {
        RUN "python3 ./tests/run_tests.py"
    }

    STEP "Verify test success" {
        ASSERT LAST_RUN.EXIT_CODE == 0
    }
}
```

### Example 2: Multi-Agent Collaboration (TissNet)
TissLang can be used to orchestrate workflows between different agents. In this example, one agent analyzes code and prepares a structured "TissLang Artifact" that can be consumed by another agent in a separate process.

```tiss
TASK "Prepare code analysis report for documentation team" {
    @persona "code_analyzer"
    @output "json"

    STEP "Read source file" {
        READ "./src/main.py" AS source_code
    }

    STEP "Identify public functions and classes" {
        PROMPT_AGENT "Extract all public function and class names from this Python code: {{source_code}}" INTO api_surface
    }

    STEP "Write TissLang Artifact for next agent" {
        # This artifact contains the result and the context for the next agent.
        WRITE "./artifacts/analysis_report.tiss" <<TISS_ARTIFACT
@source_task "Prepare code analysis report"
@source_agent "code_analyzer_v1"

TASK "Generate user-friendly documentation from API surface" {
    SET_BUDGET API_CALLS = 5

    STEP "Generate documentation" {
        @persona "technical_writer"
        PROMPT_AGENT "Write documentation for the following API surface: {{LAST_RUN_RESULT}}"
    }
}

LAST_RUN_RESULT = {{api_surface}}
TISS_ARTIFACT
    }
}
```

---

## 6. Advanced Features (Planned)

The following features are part of the TissLang roadmap and are not yet implemented.

- **`IF / ELSE`**: Conditional execution of command blocks.
- **`PAUSE`**: A general-purpose command to pause execution and wait for human input. This is less structured than `REQUEST_REVIEW`.


# TissLang IDE Native Implementation

This document outlines the refactoring process undertaken to remove the dependency on the Qt GUI framework from the TissLang IDE, replacing it with a native terminal-based interface using the `ncurses` library.

## 1. Goal

The primary objective was to refactor the C++ IDE application located in `quanta_tissu/tisslm/program/c/` to eliminate all dependencies on the Qt framework, thereby creating a lightweight, native, terminal-based editor.

## 2. Core Component Refactoring

The core graphical components of the application were rewritten to use `ncurses` for rendering and input handling.

### `main.cpp`
The application's entry point was modified to initialize and terminate the `ncurses` screen, replacing the `QApplication` loop.

### `MainWindow.h` & `MainWindow.cpp`
- The `MainWindow` class was stripped of its `QMainWindow` inheritance.
- All Qt-specific headers, signals, slots, and UI element members (menus, actions, status bar) were removed.
- The class was redesigned to manage the main `ncurses` windows: a menu bar, a status bar, and a main editor window.
- A `run()` method was implemented to contain the main application loop, handling input and dispatching draw calls.

### `TissEditor.h` & `TissEditor.cpp`
- The `TissEditor` class, formerly a `QPlainTextEdit` subclass, was converted into a plain C++ class.
- It now manages an `ncurses` window (`WINDOW*`) and a text buffer (`std::vector<std::string>`).
- Methods for handling user input (`handle_input`) and drawing the text buffer to the screen (`draw`) were implemented to provide basic text editing functionality within the terminal.

## 3. Feature Removal and Rationale

Several features were deeply integrated with the Qt framework and were removed to achieve the goal of a Qt-free application. Re-implementing them natively was deemed out of scope for the initial refactoring, especially under a no-compile/no-test mandate.

- **`SearchDialog`**: This Qt-based dialog was removed. Native search functionality would be implemented via a command-line interface in the status bar.
- **`TissSyntaxHighlighter`**: This feature relied on Qt's text formatting engine. It was removed as native `ncurses` highlighting requires a complex, language-specific parser.
- **`TissLinter`**: The linter's logic was built on Qt's data structures (`QString`, `QMap`, etc.). Both the library and its command-line interface (`cli/lint.cpp`) were removed.

## 4. Build System Changes (`CMakeLists.txt`)

The `CMakeLists.txt` file was updated to reflect the architectural changes:
- The `find_package` call for `Qt5` was removed.
- A `find_package` call for `Curses` (ncurses) was added.
- The list of source files was updated to remove the deleted component files.
- The target executable is now linked against the `ncurses` library instead of the Qt libraries.

## 5. Result

The TissLang IDE is now a native terminal application that no longer requires the Qt framework. It provides basic text editing capabilities within an `ncurses`-based interface. The codebase is significantly lighter and free of graphical server dependencies.

