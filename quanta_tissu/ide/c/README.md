# TissLang C++ IDE

This directory contains the C++ source code for a conceptual Integrated Development Environment (IDE) for TissLang.

## Purpose

This codebase serves as a blueprint for a potential C++-based IDE for editing and interacting with `.tiss` files. It was created under a "No-Compile Mandate," meaning the code is intended to be structurally sound and illustrative of a real application's architecture, but it is not compilable or functional in its current state.

The code is written to mimic the structure of an application built with a standard C++ GUI framework like Qt or wxWidgets.

## Components

The IDE is composed of the following key components:

-   `main.cpp`: The main entry point for the application. It sets up the mock application object and the main window.
-   `MainWindow.h`/`.cpp`: Defines the main window of the IDE. It is responsible for orchestrating the layout, menus, and other UI components.
-   `TissEditor.h`/`.cpp`: Defines the specialized text editor widget for TissLang. This would be the central component where users write and edit `.tiss` scripts.
-   `TissSyntaxHighlighter.h`/`.cpp`: Implements the logic for TissLang syntax highlighting. It uses regular expressions to identify keywords, comments, and strings, based on the official `TissLang_plan.md`.
-   `.gitkeep`: An empty file to ensure the `c` directory is tracked by Git.
