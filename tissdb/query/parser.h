#pragma once

#include "ast.h"
#include <string>

namespace TissDB {
namespace Query {

// The Parser class is responsible for converting a raw TissQL query string
// into an Abstract Syntax Tree (AST).
class Parser {
public:
    Parser();

    // Parses a TissQL query string.
    // If the query is valid, it returns the corresponding AST.
    // If the query is malformed, it throws a std::runtime_error.
    AST parse(const std::string& query_string);
};

} // namespace Query
} // namespace TissDB
