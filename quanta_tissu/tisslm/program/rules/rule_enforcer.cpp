#include "rule_enforcer.h"
#include <sstream>
#include <map>

namespace TissLM {
namespace Rules {

RuleEnforcer::RuleEnforcer(float strictness)
    : strictness_(strictness) {
}

std::vector<std::string> RuleEnforcer::split_into_words(const std::string& text) const {
    std::vector<std::string> words;
    std::string current_word;
    for (char c : text) {
        if (std::isspace(c) || std::ispunct(c)) {
            if (!current_word.empty()) {
                words.push_back(current_word);
                current_word.clear();
            }
        } else {
            current_word += c;
        }
    }
    if (!current_word.empty()) {
        words.push_back(current_word);
    }
    return words;
}

std::string RuleEnforcer::enforce_repetition_rule(const std::string& text) const {
    std::vector<std::string> words = split_into_words(text);
    if (words.empty()) return text;

    std::ostringstream oss;
    std::map<std::string, int> word_counts;
    std::string last_word;

    for (const auto& word : words) {
        // Simple repetition check: if current word is same as last word, skip it based on strictness
        if (word == last_word && (float)rand() / RAND_MAX < strictness_) {
            continue;
        }
        oss << word << " ";
        last_word = word;
    }
    std::string result = oss.str();
    if (!result.empty() && result.back() == ' ') {
        result.pop_back(); // Remove trailing space
    }
    return result;
}

std::string RuleEnforcer::enforce_capitalization_rule(const std::string& text) const {
    std::string result = text;
    bool capitalize_next = true;
    for (char& c : result) {
        if (capitalize_next && std::isalpha(c)) {
            c = std::toupper(c);
            capitalize_next = false;
        } else if (c == '.' || c == '!' || c == '?') {
            capitalize_next = true;
        }
    }
    return result;
}

std::string RuleEnforcer::apply_rules(const std::string& text) {
    std::string cleaned_text = text;
    cleaned_text = enforce_repetition_rule(cleaned_text);
    cleaned_text = enforce_capitalization_rule(cleaned_text);
    return cleaned_text;
}

} // namespace Rules
} // namespace TissLM
