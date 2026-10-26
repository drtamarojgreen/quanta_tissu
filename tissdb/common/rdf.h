#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace TissDB {
namespace Rdf {

// Well-known vocabulary IRIs.
namespace Vocab {
inline constexpr const char* RDF_NS = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
inline constexpr const char* RDF_TYPE = "http://www.w3.org/1999/02/22-rdf-syntax-ns#type";
inline constexpr const char* RDF_LANG_STRING = "http://www.w3.org/1999/02/22-rdf-syntax-ns#langString";
} // namespace Vocab

// XML Schema datatypes commonly used for RDF literals.
namespace XSD {
inline constexpr const char* NS = "http://www.w3.org/2001/XMLSchema#";
inline constexpr const char* STRING = "http://www.w3.org/2001/XMLSchema#string";
inline constexpr const char* BOOLEAN = "http://www.w3.org/2001/XMLSchema#boolean";
inline constexpr const char* INTEGER = "http://www.w3.org/2001/XMLSchema#integer";
inline constexpr const char* DECIMAL = "http://www.w3.org/2001/XMLSchema#decimal";
inline constexpr const char* DOUBLE = "http://www.w3.org/2001/XMLSchema#double";
inline constexpr const char* FLOAT = "http://www.w3.org/2001/XMLSchema#float";
inline constexpr const char* DATE = "http://www.w3.org/2001/XMLSchema#date";
inline constexpr const char* DATE_TIME = "http://www.w3.org/2001/XMLSchema#dateTime";
} // namespace XSD

enum class TermType {
    IRI,
    BLANK_NODE,
    LITERAL
};

// An RDF term: an IRI, a blank node, or a literal.
// Literals always carry a datatype IRI. A literal without an explicit datatype is
// an xsd:string, and a language-tagged literal is an rdf:langString (RDF 1.1).
struct Term {
    TermType type = TermType::IRI;
    std::string value;     // IRI, blank node label (without "_:"), or literal lexical form
    std::string datatype;  // Literal datatype IRI; empty for IRIs and blank nodes
    std::string language;  // Lowercased language tag; empty unless rdf:langString

    static Term iri(const std::string& iri);
    static Term blank(const std::string& label);
    static Term literal(const std::string& lexical, const std::string& datatype = XSD::STRING);
    static Term lang_literal(const std::string& lexical, const std::string& language);

    bool is_iri() const { return type == TermType::IRI; }
    bool is_blank() const { return type == TermType::BLANK_NODE; }
    bool is_literal() const { return type == TermType::LITERAL; }

    // True for literals whose datatype is one of the XSD numeric types.
    bool is_numeric() const;
    // Numeric value of a numeric literal, or nullopt if not numeric / not parseable.
    std::optional<double> numeric_value() const;

    // Canonical N-Triples encoding of this term (e.g. <http://a>, _:b0, "x"@en).
    std::string to_ntriples() const;

    bool operator==(const Term& other) const;
    bool operator!=(const Term& other) const { return !(*this == other); }
    bool operator<(const Term& other) const;
};

// An RDF triple (subject, predicate, object).
struct Triple {
    Term subject;
    Term predicate;
    Term object;

    // N-Triples line for this triple, without trailing newline.
    std::string to_ntriples() const;

    bool operator==(const Triple& other) const;
    bool operator!=(const Triple& other) const { return !(*this == other); }
    bool operator<(const Triple& other) const;
};

// Escapes a literal lexical form for use between double quotes in N-Triples.
std::string escape_literal(const std::string& lexical);
// Escapes characters that are not allowed inside an N-Triples IRIREF.
std::string escape_iri(const std::string& iri);
// Appends the UTF-8 encoding of a Unicode code point to out.
void append_utf8(std::string& out, uint32_t code_point);

} // namespace Rdf
} // namespace TissDB
