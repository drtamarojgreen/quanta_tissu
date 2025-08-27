#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>

// Forward declarations for child widgets to avoid circular dependencies.
// In a real GUI framework, these would be pointers to actual widget classes.
class TissEditor;
class TissSyntaxHighlighter;

// Represents the main window of the TissLang IDE.
// In a real application, this class would inherit from a base window class
// like QMainWindow (Qt) or wxFrame (wxWidgets).
class MainWindow {
public:
    // Constructor and Destructor
    MainWindow();
    ~MainWindow();

private:
    // In a real IDE, these would be pointers to the actual widgets.
    TissEditor* editor;
    TissSyntaxHighlighter* syntax_highlighter;

    // --- Private Helper Functions ---
    // These functions would be called by the constructor to set up the UI.
    void setupUI();
    void createMenus();
    void createActions();
    void createStatusBar();
};

#endif // MAINWINDOW_H
