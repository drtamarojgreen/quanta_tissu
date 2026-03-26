#include "rule_enforcer.h"
#include <sstream>
#include <map>
#include <algorithm>

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
    std::string last_word;
    size_t repeat_count = 0;

    for (const auto& word : words) {
        std::string lower_word = word;
        std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);

        if (lower_word == last_word) {
            repeat_count++;
            // Penalty based on repeat count and strictness
            if (repeat_count > 1 && strictness_ > 0.5f) continue;
            if (repeat_count > 0 && strictness_ > 0.8f) continue;
        } else {
            repeat_count = 0;
        }

        oss << word << " ";
        last_word = lower_word;
    }
    std::string result = oss.str();
    if (!result.empty() && result.back() == ' ') result.pop_back();
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

}
}
