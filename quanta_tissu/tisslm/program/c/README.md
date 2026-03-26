# TissLang C++ IDE

This directory contains the C++ source code for an Integrated Development Environment (IDE) for TissLang.

## Components

The IDE is composed of the following key components:

-   `main.cpp`: The main entry point for the application. It sets up the application object and the main window.
-   `MainWindow.h`/`.cpp`: Defines the main window of the IDE. It is responsible for orchestrating the layout, menus, and other UI components.
-   `TissEditor.h`/`.cpp`: Defines the specialized text editor widget for TissLang. This is the central component where users write and edit `.tiss` scripts.
-   `TissSyntaxHighlighter.h`/`.cpp`: Implements the logic for TissLang syntax highlighting. It uses regular expressions to identify keywords, comments, and strings.
-   `SearchDialog.h`/`.cpp`: Implements a find and replace dialog with support for search macros.
-   `TissLinter.h`/`.cpp`: Provides static analysis and linting for `.tiss` files.
