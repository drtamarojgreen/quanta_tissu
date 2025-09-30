#include <iostream>

// Function declarations for tests
void run_parser_tests();

int main() {
    try {
        run_parser_tests();
    } catch (const std::exception& e) {
        std::cerr << "A test failed with an exception: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}
