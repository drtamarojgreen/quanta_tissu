#include "pre_tokenizer.h"
#include <vector>
#include <string>

namespace TissLM {
namespace Tokenizer {

// Helper function to check if a character is part of a bilingual word (English/Spanish)
// Note: This is a simplified check and may not cover all Unicode cases perfectly.
bool is_bilingual_alpha(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    // Basic ASCII letters
    if ((uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z')) {
        return true;
    }
    // Check for common 2-byte UTF-8 characters in Spanish
    if (uc >= 0xc3) { // Most Spanish characters start with 0xc3
        return true;
    }
    return false;
}

// Helper function to check if a character is a digit
bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

// Helper function to check if a character is whitespace
bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

std::vector<std::string> pre_tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    if (text.empty()) {
        return tokens;
    }

    const std::vector<std::string> contractions = {
        "'s", "'t", "'re", "'ve", "'m", "'ll", "'d"
    };

    size_t i = 0;
    while (i < text.length()) {
        // 1. Handle contractions in a robust way
        bool contraction_found = false;
        for (const auto& contraction : contractions) {
            if (text.substr(i, contraction.length()) == contraction) {
                tokens.push_back(contraction);
                i += contraction.length();
                contraction_found = true;
                break;
            }
        }
        if (contraction_found) {
            continue;
        }

        // 2. Handle words (English and Spanish)
        if (is_bilingual_alpha(text[i])) {
            size_t j = i;
            // This simple check works because UTF-8 multi-byte characters for Spanish
            // will not match the other categories (digits, whitespace, etc.)
            while (j < text.length() && is_bilingual_alpha(text[j])) {
                j++;
            }
            tokens.push_back(text.substr(i, j - i));
            i = j;
            continue;
        }

        // 3. Handle numbers
        if (is_digit(text[i])) {
            size_t j = i;
            while (j < text.length() && is_digit(text[j])) {
                j++;
            }
            tokens.push_back(text.substr(i, j - i));
            i = j;
            continue;
        }

        // 4. Handle whitespace
        if (is_whitespace(text[i])) {
            size_t j = i;
            while (j < text.length() && is_whitespace(text[j])) {
                j++;
            }
            tokens.push_back(text.substr(i, j - i));
            i = j;
            continue;
        }

        // 5. Handle single characters (punctuation/symbols)
        tokens.push_back(text.substr(i, 1));
        i++;
    }

    return tokens;
}

} // namespace Tokenizer
} // namespace TissLM