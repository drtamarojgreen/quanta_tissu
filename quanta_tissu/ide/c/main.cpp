#include "MainWindow.h"
#include <iostream>

// This would be a platform-specific Application class from a GUI framework
// (e.g., QApplication in Qt, wxApp in wxWidgets).
// We are creating a mock class here for demonstration purposes.
class MockApplication {
public:
    MockApplication(int &argc, char **argv) {
        std::cout << "MockApplication: Initializing with command line arguments." << std::endl;
    }

    int exec() {
        std::cout << "MockApplication: Starting the event loop." << std::endl;
        // In a real application, this would block and process GUI events.
        // Here, we just simulate it running and then exiting.
        std::cout << "MockApplication: Event loop finished." << std::endl;
        return 0;
    }
};

int main(int argc, char *argv[]) {
    // --- Application Initialization ---
    // In a real GUI application, this object manages the application's main event loop.
    MockApplication app(argc, argv);

    // --- Main Window Creation ---
    // Create the main window of the IDE.
    MainWindow main_window;

    // --- Show the Window ---
    // In a real GUI app, this would make the window visible.
    // main_window.show();
    std::cout << "main: Instructing MainWindow to show." << std::endl;


    // --- Start Event Loop ---
    // This call starts the application's event loop, which waits for user interaction.
    // The application will exit when the event loop terminates (e.g., when the main window is closed).
    return app.exec();
}
