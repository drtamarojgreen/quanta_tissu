#include "rdf.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <tuple>

namespace TissDB {
namespace Rdf {

namespace {

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

bool is_numeric_datatype(const std::string& datatype) {
    static const char* const numeric_types[] = {
        "integer", "decimal", "double", "float",
        "long", "int", "short", "byte",
        "nonNegativeInteger", "nonPositiveInteger", "positiveInteger", "negativeInteger",
        "unsignedLong", "unsignedInt", "unsignedShort", "unsignedByte"};
    const std::string ns = XSD::NS;
    if (datatype.compare(0, ns.size(), ns) != 0) return false;
    const std::string local = datatype.substr(ns.size());
    for (const char* t : numeric_types) {
        if (local == t) return true;
    }
    return false;
}

} // anonymous namespace

Term Term::iri(const std::string& iri) {
    Term t;
    t.type = TermType::IRI;
    t.value = iri;
    return t;
}

Term Term::blank(const std::string& label) {
    Term t;
    t.type = TermType::BLANK_NODE;
    t.value = label;
    return t;
}

Term Term::literal(const std::string& lexical, const std::string& datatype) {
    Term t;
    t.type = TermType::LITERAL;
    t.value = lexical;
    t.datatype = datatype.empty() ? XSD::STRING : datatype;
    return t;
}

Term Term::lang_literal(const std::string& lexical, const std::string& language) {
    Term t;
    t.type = TermType::LITERAL;
    t.value = lexical;
    t.datatype = Vocab::RDF_LANG_STRING;
    t.language = to_lower(language);
    return t;
}

bool Term::is_numeric() const {
    return type == TermType::LITERAL && is_numeric_datatype(datatype);
}

std::optional<double> Term::numeric_value() const {
    if (!is_numeric() || value.empty()) return std::nullopt;
    const char* begin = value.c_str();
    char* end = nullptr;
    double d = std::strtod(begin, &end);
    if (end != begin + value.size()) return std::nullopt;
    return d;
}

std::string Term::to_ntriples() const {
    switch (type) {
        case TermType::IRI:
            return "<" + escape_iri(value) + ">";
        case TermType::BLANK_NODE:
            return "_:" + value;
        case TermType::LITERAL: {
            std::string out = "\"" + escape_literal(value) + "\"";
            if (!language.empty()) {
                out += "@" + language;
            } else if (datatype != XSD::STRING) {
                out += "^^<" + escape_iri(datatype) + ">";
            }
            return out;
        }
    }
    return "";
}

bool Term::operator==(const Term& other) const {
    return type == other.type && value == other.value && datatype == other.datatype && language == other.language;
}

bool Term::operator<(const Term& other) const {
    return std::tie(type, value, datatype, language) < std::tie(other.type, other.value, other.datatype, other.language);
}

std::string Triple::to_ntriples() const {
    return subject.to_ntriples() + " " + predicate.to_ntriples() + " " + object.to_ntriples() + " .";
}

bool Triple::operator==(const Triple& other) const {
    return subject == other.subject && predicate == other.predicate && object == other.object;
}

bool Triple::operator<(const Triple& other) const {
    return std::tie(subject, predicate, object) < std::tie(other.subject, other.predicate, other.object);
}

std::string escape_literal(const std::string& lexical) {
    std::string out;
    out.reserve(lexical.size());
    for (char c : lexical) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            default: out += c; break;
        }
    }
    return out;
}

std::string escape_iri(const std::string& iri) {
    std::string out;
    out.reserve(iri.size());
    for (char ch : iri) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (c <= 0x20 || c == '<' || c == '>' || c == '"' || c == '{' || c == '}' ||
            c == '|' || c == '^' || c == '`' || c == '\\') {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "\\u%04X", c);
            out += buf;
        } else {
            out += ch;
        }
    }
    return out;
}

void append_utf8(std::string& out, uint32_t cp) {
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else if (cp < 0x800) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (cp >> 18));
        out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

} // namespace Rdf
} // namespace TissDB
