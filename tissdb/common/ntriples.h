#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "rdf.h"

namespace TissDB {
namespace NTriples {

// Thrown when N-Triples input is malformed. line() is 1-based, or 0 when the
// error is not tied to a line of a document (e.g. parse_term).
class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message, size_t line)
        : std::runtime_error(line ? "N-Triples parse error on line " + std::to_string(line) + ": " + message
                                  : "N-Triples parse error: " + message),
          line_(line) {}
    size_t line() const { return line_; }

private:
    size_t line_;
};

// Parses an N-Triples document (one triple per line, '#' comments, blank lines).
std::vector<Rdf::Triple> parse(const std::string& document);

// Parses a single N-Triples line. Returns nullopt for blank or comment-only lines.
std::optional<Rdf::Triple> parse_line(const std::string& line, size_t line_number = 0);

// Parses a single term in N-Triples syntax, e.g. <http://a>, _:b, "x"^^<http://dt>.
Rdf::Term parse_term(const std::string& text);

// Serializes triples as an N-Triples document, one line per triple.
std::string serialize(const std::vector<Rdf::Triple>& triples);

} // namespace NTriples
} // namespace TissDB
