#pragma once

#include "ast.h"
#include <string>
#include <memory>
#include <vector>

namespace TissLang {

// The Parser class is responsible for turning a string of TissLang source code
// into an Abstract Syntax Tree (AST), represented by a TissLang::Script object.
class Parser {
public:
    // Constructor takes the source code to be parsed.
    explicit Parser(std::string source_code);

    // The main entry point for parsing. Returns the root of the AST.
    // Throws a std::runtime_error on parsing failure.
    std::unique_ptr<Script> parse();

private:
    std::string source;
    std::vector<std::string> lines;
    size_t current_line_index = 0;

    // --- High-level parsing functions ---
    void parse_script_header(Script& script);
    void parse_task_declaration(Script& script);
    void parse_all_steps(Script& script);
    std::unique_ptr<Step> parse_step_block();
    void parse_commands_in_step(Step& step);
    std::unique_ptr<Command> parse_command_line(const std::string& line);

    // --- Low-level utility functions ---

    // Safely gets the current line without advancing. Returns an empty string if at the end.
    std::string peek_line();
    // Gets the current line and advances the line index.
    std::string advance_line();
    // Checks if we have processed all lines.
    bool is_at_end();
    // Trims whitespace from both ends of a string.
    std::string trim(const std::string& s);
};

} // namespace TissLang
