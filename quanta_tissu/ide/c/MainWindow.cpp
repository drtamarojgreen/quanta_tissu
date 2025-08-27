#include "MainWindow.h"
#include "TissEditor.h"
#include "TissSyntaxHighlighter.h"
#include <iostream>

MainWindow::MainWindow() {
    // This constructor would initialize the main window.
    std::cout << "MainWindow: Constructor called." << std::endl;
    setupUI();
}

MainWindow::~MainWindow() {
    // The destructor would clean up allocated resources.
    // In a real application with pointers, we would delete them here.
    // delete editor;
    // delete syntax_highlighter;
    std::cout << "MainWindow: Destructor called." << std::endl;
}

void MainWindow::setupUI() {
    // This function would assemble the UI components of the main window.
    std::cout << "MainWindow: Setting up UI..." << std::endl;

    // 1. Create the text editor widget.
    // editor = new TissEditor(this); // 'this' would be the parent widget.
    std::cout << "MainWindow: Creating TissEditor widget." << std::endl;

    // 2. Create the syntax highlighter and attach it to the editor.
    // syntax_highlighter = new TissSyntaxHighlighter(editor->document());
    std::cout << "MainWindow: Creating TissSyntaxHighlighter." << std::endl;

    // 3. Set the central widget of the main window.
    // setCentralWidget(editor);
    std::cout << "MainWindow: Setting editor as the central widget." << std::endl;


    // 4. Create menus and status bar.
    createActions();
    createMenus();
    createStatusBar();

    std::cout << "MainWindow: UI setup complete." << std::endl;
}

void MainWindow::createActions() {
    // In a real app, this would create QAction or wxMenuItem objects
    // for menu items like "File > Open" or "Edit > Copy".
    std::cout << "MainWindow: Creating UI actions (e.g., Open, Save, Exit)." << std::endl;
}

void MainWindow::createMenus() {
    // This would create the main menu bar (File, Edit, View, etc.)
    // and populate it with the actions created above.
    std::cout << "MainWindow: Creating menu bar." << std::endl;
}

void MainWindow::createStatusBar() {
    // This would create a status bar at the bottom of the window.
    // statusBar()->showMessage("Ready");
    std::cout << "MainWindow: Creating status bar." << std::endl;
}
