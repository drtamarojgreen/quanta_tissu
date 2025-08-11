#include "graph_logic.h"
#include <iostream>

/**
 * @brief Main function for the graph visualization application.
 *
 * Creates an instance of the GraphLogic class and runs the application.
 * @return int Exit code. Returns 0 on successful execution.
 */
int main() {
    GraphLogic app;
    app.run();
    std::cout << "Exiting application." << std::endl;
    return 0;
}