# TissLang IDE Development Comparison

This document provides a detailed comparison between the two TissLang IDE implementations found in this repository: the C++ GUI blueprint and the Python command-line tool.

## 1. C++ IDE (`quanta_tissu/ide/c/`)

### Overview

The C++ IDE is a **conceptual blueprint** for a full-featured, graphical user interface (GUI) application intended for TissLang development. The code is structured to mimic a real-world desktop application using a framework like Qt, but it is non-compilable by design, as per the project's "No-Compile Mandate."

Its primary purpose is to serve as an architectural model for a robust, interactive TissLang editor.

### Features

-   **Graphical User Interface:** A complete desktop application window with menus, a central text editor, and a status bar.
-   **Interactive Text Editor:** A central widget for writing and editing `.tiss` code.
-   **File Management:** Standard file operations are supported through a menu system:
    -   New File
    -   Open File
    -   Save
    -   Save As
-   **Syntax Highlighting:** Real-time syntax highlighting is applied directly in the editor as the user types.
-   **Linter Integration:** A linter runs automatically when a file is opened or saved, with debugging output to show where issues are found.
-   **Find and Replace:** A fully interactive dialog for searching and replacing text. It supports:
    -   Case-sensitive and insensitive search.
    -   Regular expressions.
    -   Find Next / Find Previous.
    -   Replace / Replace All.
-   **Standard Edit Operations:** Includes standard menu actions for Cut, Copy, and Paste.
-   **Session Persistence:** The IDE remembers its window size and position between sessions.

## 2. Python IDE (`quanta_tissu/ide/p/`)

### Overview

The Python IDE is a lightweight **command-line interface (CLI) tool** for inspecting and processing TissLang files. It is not an interactive editor but rather a utility that reads a file, analyzes it, and prints the processed output to the console.

Its primary purpose is to provide a quick way to validate and review `.tiss` scripts from the terminal.

### Features

-   **Command-Line Interface:** The tool is run from the terminal, taking a file path as an argument.
-   **File Viewer:** It reads a `.tiss` file and prints its contents to the standard output.
-   **Console-Based Syntax Highlighting:** It applies color codes to the output to simulate syntax highlighting in the terminal.
-   **Console-Based Linting:** It runs a linter on the file and prints any errors directly below the line of code where they occur.
-   **Non-Interactive Search and Replace:** It provides command-line flags (`--replace <regex> <replacement>`) to perform a one-time search and replace operation on a file, modifying it directly on disk.

## 3. Feature Comparison

| Feature                 | C++ IDE (GUI Blueprint)                                  | Python IDE (CLI Tool)                                     |
| ----------------------- | -------------------------------------------------------- | --------------------------------------------------------- |
| **Interface**           | Graphical User Interface (GUI)                           | Command-Line Interface (CLI)                              |
| **Editing**             | Interactive, real-time text editing                      | None (File is processed and printed)                      |
| **File Operations**     | Menu-driven (New, Open, Save)                            | Reads a file specified by a command-line argument       |
| **Syntax Highlighting** | Real-time, within the editor                             | Applied to console output                                 |
| **Linter**              | Runs on load/save; output to debug console               | Runs on execution; errors printed below the code line     |
| **Find/Replace**        | Interactive dialog with regex support                    | Non-interactive, via command-line arguments             |
| **User Experience**     | Rich, interactive, designed for active development       | Lightweight, scriptable, designed for quick inspection    |
