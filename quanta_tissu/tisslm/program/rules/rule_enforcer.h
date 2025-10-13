#ifndef TISSLM_RULE_ENFORCER_H
#define TISSLM_RULE_ENFORCER_H

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cctype>

namespace TissDB {
namespace TissLM {
namespace Core {

class RuleEnforcer {
public:
    RuleEnforcer(float strictness = 0.5f);

    std::string apply_rules(const std::string& text);

private:
    float strictness_;

    // Rule implementations
    std::string enforce_repetition_rule(const std::string& text) const;
    std::string enforce_capitalization_rule(const std::string& text) const;

    // Helper to split text into words
    std::vector<std::string> split_into_words(const std::string& text) const;
};

} // namespace Core
} // namespace TissLM
} // namespace TissDB

#endif // TISSLM_RULE_ENFORCER_H
