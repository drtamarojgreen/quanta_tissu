#include "pre_tokenizer.h"
#include <vector>
#include <string>

namespace TissLM {
namespace Tokenizer {

/**
 * @brief Checks if a character belongs to a bilingual alphabet (English/Spanish).
 * Handles standard ASCII and common UTF-8 lead bytes for Spanish accents.
 */
bool is_bilingual_alpha(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    if ((uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z')) return true;
    if (uc >= 0xc2 && uc <= 0xc3) return true; // Start of UTF-8 sequences for common accents
    if (uc >= 0x80 && uc <= 0xbf) return true; // UTF-8 continuation bytes
    return false;
}

bool is_digit(char c) { return c >= '0' && c <= '9'; }
bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

std::vector<std::string> pre_tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    if (text.empty()) return tokens;

    const std::vector<std::string> contractions = {"'s", "'t", "'re", "'ve", "'m", "'ll", "'d"};

    size_t i = 0;
    while (i < text.length()) {
        bool contraction_found = false;
        for (const auto& c : contractions) {
            if (i + c.length() <= text.length() && text.substr(i, c.length()) == c) {
                tokens.push_back(c); i += c.length(); contraction_found = true; break;
            }
        }
        if (contraction_found) continue;

        if (is_bilingual_alpha(text[i])) {
            size_t j = i;
            while (j < text.length() && is_bilingual_alpha(text[j])) j++;
            tokens.push_back(text.substr(i, j - i)); i = j; continue;
        }

        if (is_digit(text[i])) {
            size_t j = i;
            while (j < text.length() && is_digit(text[j])) j++;
            tokens.push_back(text.substr(i, j - i)); i = j; continue;
        }

        if (is_whitespace(text[i])) {
            size_t j = i;
            while (j < text.length() && is_whitespace(text[j])) j++;
            tokens.push_back(text.substr(i, j - i)); i = j; continue;
        }

        tokens.push_back(text.substr(i, 1)); i++;
    }
    return tokens;
}

}
}
