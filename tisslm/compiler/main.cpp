#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

// The main entry point for the TissLang compiler/interpreter.
//
// This application will take one argument: the path to a .tiss script file.
// It will then perform the following steps:
// 1. Read the content of the specified file.
// 2. Instantiate the TissLang::Parser with the file content.
// 3. Call the parser's parse() method to generate an Abstract Syntax Tree (AST).
// 4. Print a success message and some basic information from the AST.
// 5. In a future phase, this AST would be passed to an ExecutionEngine.
//
// Note: As per the project constraints, this code will not be compiled or run
// by the agent. It is created to define the structure of the C++ application.

int main(int argc, char* argv[]) {
    // Check for the correct number of command-line arguments.
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path/to/script.tiss>" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::ifstream file_stream(filepath);
    if (!file_stream) {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return 1;
    }

    // Read the entire file into a string buffer.
    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    std::string source_code = buffer.str();

    std::cout << "Parsing TissLang script: " << filepath << std::endl;

    try {
        // Create a parser instance with the source code.
        auto parser = TissLang::Parser(source_code);
        // Parse the source to get the AST.
        std::unique_ptr<TissLang::Script> ast = parser.parse();

        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Parsing successful!" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Task: " << ast->task_description << std::endl;
        if (!ast->global_language_hint.empty()) {
            std::cout << "Language Hint: " << ast->global_language_hint << std::endl;
        }
        std::cout << "Found " << ast->steps.size() << " steps." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error during parsing: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
