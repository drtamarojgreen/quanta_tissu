# TissLang IDE Development

This document provides an overview of the Integrated Development Environments (IDEs) available for TissLang.

## Python IDE (Command-Line Syntax Highlighter)

The Python IDE is a lightweight, command-line tool for viewing TissLang (`.tiss`) files with syntax highlighting.

**Location:** `quanta_tissu/ide/p/ide.py`

### Functionality

- **Syntax Highlighting:** The script reads a `.tiss` file and prints its contents to the terminal, using ANSI escape codes to colorize the syntax.
- **Language Support:** It recognizes basic TissLang constructs:
    - **Keywords:** `TASK`, `STEP`, `WRITE`, `RUN`, `ASSERT`, etc. (colored yellow)
    - **Strings:** Double-quoted strings (colored green)
    - **Comments:** Lines starting with `#` (colored grey)
    - **Directives:** Identifiers starting with `@` (colored magenta)
    - **Heredocs:** Multi-line blocks starting with `<<DELIMITER` (colored green)
- **Usage:** The tool is run from the command line and requires a single argument: the path to the `.tiss` file.

### How to Run

```bash
python quanta_tissu/ide/p/ide.py <path/to/your/file.tiss>
```

### Implementation Details

- The script uses Python's `re` module to identify language tokens with regular expressions.
- It processes the file line by line, applying highlighting rules before printing to standard output.
- It includes basic error handling for file existence and correct command-line arguments.

## C++ IDE (Conceptual GUI Application)

The C++ IDE is a non-functional, conceptual mock-up of a full-featured, graphical desktop application for TissLang development. It serves as a blueprint for a more robust development tool.

**Location:** `quanta_tissu/ide/c/`

### Architecture

The application is designed with a structure similar to one built with the Qt framework and consists of three main components:

- **`MainWindow`**: The main application window, intended to host menus, a status bar, and the editor.
- **`TissEditor`**: A custom text editor widget designed specifically for TissLang. It's planned to have features like line numbering.
- **`TissSyntaxHighlighter`**: The core engine for syntax highlighting. It uses C++ regular expressions to identify and format TissLang syntax.

### Functionality (Conceptual)

- **Graphical Interface:** Unlike the Python tool, this is designed as a windowed application, providing a more user-friendly environment.
- **Advanced Language Support:** The C++ syntax highlighter recognizes a more comprehensive set of TissLang keywords than the Python version, including control flow (`IF`, `ELSE`, `TRY`, `CATCH`) and pragmas (`#TISS!`).
- **Extensibility:** The class-based architecture is designed to be extensible, allowing for new features like code folding, auto-completion, and project management to be added in the future.

### Implementation Status

- The current code is a **non-functional mock-up**.
- It does not link against any real GUI library (like Qt or wxWidgets).
- The classes log their actions to `stdout` (e.g., "Constructor called", "Initializing syntax rules") to demonstrate the intended behavior without rendering a UI.
- The core logic is found in `TissSyntaxHighlighter.cpp`, which defines the `std::regex` patterns for the TissLang grammar.
