#include <iostream>

// A minimal App class for universal application purposes.
class App {
public:
    App();
    void printHelp();
};

// App constructor, which calls printHelp() on initialization.
App::App() {
    printHelp();
}

// The printHelp() function provides generic command info.
void App::printHelp() {
    std::cout << R"---(
--- Application Commands ---
  - 'help':   Show this list of commands.
  - 'version': Show the application version.
  - 'quit':    Exit the application.
--------------------------

)---";
}

// A simple main function to demonstrate the App class.
int main() {
    App myApp;
    // The constructor already prints the help text.
    // In a real app, there would be a command loop here.
    return 0;
}
