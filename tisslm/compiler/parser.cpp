#include "parser.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>

namespace TissLang {

// --- Utility Functions ---

// Checks if a string starts with a given prefix.
bool starts_with(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

// Trims leading and trailing whitespace from a string.
std::string Parser::trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return s;
    }
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, (last - first + 1));
}

// --- Parser Implementation ---

Parser::Parser(std::string source_code) : source(std::move(source_code)) {
    std::stringstream ss(source);
    std::string line;
    while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
    }
}

std::unique_ptr<Script> Parser::parse() {
    auto script = std::make_unique<Script>(""); // Placeholder description

    parse_script_header(*script);
    parse_task_declaration(*script);
    parse_all_steps(*script);

    if (!is_at_end()) {
        // This could happen if there's text after the last step block that is not a new step
        // For now, we will just ignore it, but a stricter parser might throw an error.
    }

    return script;
}

// --- High-level parsing functions ---

void Parser::parse_script_header(Script& script) {
    while (!is_at_end()) {
        std::string line = trim(peek_line());
        if (line.empty() || starts_with(line, "#")) {
            if (starts_with(line, "#TISS!")) {
                // Example: #TISS! Language=Python
                size_t pos = line.find("Language=");
                if (pos != std::string::npos) {
                    script.global_language_hint = trim(line.substr(pos + 9));
                }
            }
            advance_line(); // Consume comment or empty line
        } else {
            // Reached the first non-comment/empty line
            break;
        }
    }
}

void Parser::parse_task_declaration(Script& script) {
    std::string line = trim(advance_line());
    if (!starts_with(line, "TASK")) {
        throw std::runtime_error("Expected TASK declaration.");
    }
    size_t start = line.find('"');
    size_t end = line.rfind('"');
    if (start == std::string::npos || end == start) {
        throw std::runtime_error("Invalid TASK description format.");
    }
    script.task_description = line.substr(start + 1, end - start - 1);
}

void Parser::parse_all_steps(Script& script) {
     while (!is_at_end()) {
        std::string line = trim(peek_line());
        if (line.empty() || starts_with(line, "#")) {
            advance_line();
            continue;
        }
        if (starts_with(line, "STEP")) {
            script.steps.push_back(parse_step_block());
        } else {
            // If the line is not empty, not a comment, and not a STEP, it's an error.
            if (!line.empty()) {
                throw std::runtime_error("Unexpected content outside of a STEP block: " + line);
            }
            advance_line();
        }
    }
}

std::unique_ptr<Step> Parser::parse_step_block() {
    std::string line = trim(advance_line());
    if (!starts_with(line, "STEP")) {
        throw std::runtime_error("Expected STEP block.");
    }

    size_t start_quote = line.find('"');
    size_t end_quote = line.rfind('"');
    if (start_quote == std::string::npos || end_quote == start_quote) {
        throw std::runtime_error("Invalid STEP description format.");
    }
    std::string description = line.substr(start_quote + 1, end_quote - start_quote - 1);
    auto step = std::make_unique<Step>(description);

    // Expect an opening brace
    if (trim(advance_line()) != "{") {
        throw std::runtime_error("Expected '{' to begin STEP block.");
    }

    // Parse commands within the block
    parse_commands_in_step(*step);

    return step;
}

void Parser::parse_commands_in_step(Step& step) {
    while (!is_at_end()) {
        std::string line = trim(peek_line());
        if (line == "}") {
            advance_line(); // Consume the closing brace
            return;
        }
        if (line.empty() || starts_with(line, "#")) {
            advance_line();
            continue;
        }
        step.commands.push_back(parse_command_line(line));
        // Note: parse_command_line is responsible for advancing past its own lines
    }
    // If we run out of lines before finding '}', the script is malformed.
    throw std::runtime_error("Expected '}' to end STEP block, but reached end of file.");
}

std::unique_ptr<Command> Parser::parse_command_line(const std::string& line) {
    std::string trimmed_line = trim(line);
    advance_line(); // Consume the command line itself

    if (starts_with(trimmed_line, "WRITE")) {
        // e.g., WRITE "main.py" <<PYTHON
        size_t first_quote = trimmed_line.find('"');
        size_t second_quote = trimmed_line.find('"', first_quote + 1);
        if (first_quote == std::string::npos || second_quote == std::string::npos) {
            throw std::runtime_error("Invalid WRITE syntax: missing filepath.");
        }
        std::string filepath = trimmed_line.substr(first_quote + 1, second_quote - first_quote - 1);

        std::string delimiter;
        size_t delimiter_pos = trimmed_line.find("<<");
        if (delimiter_pos != std::string::npos) {
            delimiter = trim(trimmed_line.substr(delimiter_pos + 2));
        } else {
             throw std::runtime_error("Invalid WRITE syntax: missing '<<' for heredoc.");
        }

        // Read heredoc content
        std::string content;
        while (!is_at_end()) {
            std::string content_line = advance_line(); // Don't trim, preserve indentation
            if (trim(content_line) == delimiter) {
                // Remove trailing newline if it exists from the last line before delimiter
                if (!content.empty() && content.back() == '\n') {
                    content.pop_back();
                }
                return std::make_unique<WriteCommand>(filepath, content, delimiter);
            }
            content += content_line + "\n";
        }
        throw std::runtime_error("Unterminated WRITE block: delimiter not found.");

    } else if (starts_with(trimmed_line, "RUN")) {
        // e.g., RUN "python main.py"
        size_t first_quote = trimmed_line.find('"');
        size_t second_quote = trimmed_line.rfind('"');
        if (first_quote == std::string::npos || second_quote == first_quote) {
            throw std::runtime_error("Invalid RUN syntax: command must be in quotes.");
        }
        std::string command = trimmed_line.substr(first_quote + 1, second_quote - first_quote - 1);
        return std::make_unique<RunCommand>(command);

    } else if (starts_with(trimmed_line, "ASSERT")) {
        // e.g., ASSERT LAST_RUN.EXIT_CODE == 0
        std::string condition = trim(trimmed_line.substr(6)); // 6 is len("ASSERT ")
        return std::make_unique<AssertCommand>(condition);
    }

    throw std::runtime_error("Unknown command: " + trimmed_line);
}


// --- Low-level utility functions ---

std::string Parser::peek_line() {
    if (is_at_end()) {
        return "";
    }
    return lines[current_line_index];
}

std::string Parser::advance_line() {
    if (is_at_end()) {
        return "";
    }
    return lines[current_line_index++];
}

bool Parser::is_at_end() {
    return current_line_index >= lines.size();
}

} // namespace TissLang
