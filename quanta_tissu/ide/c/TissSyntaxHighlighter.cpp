#include "TissSyntaxHighlighter.h"
#include <iostream>

TissSyntaxHighlighter::TissSyntaxHighlighter() : MockSyntaxHighlighter() {
    std::cout << "TissSyntaxHighlighter: Constructor called." << std::endl;
    initializeRules();
}

TissSyntaxHighlighter::~TissSyntaxHighlighter() {
    std::cout << "TissSyntaxHighlighter: Destructor called." << std::endl;
}

void TissSyntaxHighlighter::initializeRules() {
    std::cout << "TissSyntaxHighlighter: Initializing syntax rules." << std::endl;
    HighlightingRule rule;

    // Rule for TissLang keywords.
    // The pattern uses word boundaries (\\b) to avoid matching parts of words.
    rule.pattern = std::regex("\\b(TASK|STEP|SETUP|READ|WRITE|RUN|ASSERT|AS|CONTAINS|IS_EMPTY|EXIT_CODE|LAST_RUN|STDOUT|STDERR|FILE|EXISTS|IF|ELSE|DEFINE_TASK|TRY|CATCH|PAUSE|REQUEST_REVIEW|CHOOSE|OPTION|ESTIMATE_COST|SET_BUDGET|PROMPT_AGENT|INTO)\\b");
    rule.format_name = "keyword";
    highlighting_rules.push_back(rule);

    // Rule for single-line comments (from '#' to the end of the line).
    rule.pattern = std::regex("#[^\n]*");
    rule.format_name = "comment";
    highlighting_rules.push_back(rule);

    // Rule for strings enclosed in double quotes.
    rule.pattern = std::regex("\"[^\"]*\"");
    rule.format_name = "string";
    highlighting_rules.push_back(rule);

    // Rule for the special #TISS! pragma
    rule.pattern = std::regex("^#TISS!.*");
    rule.format_name = "pragma";
    highlighting_rules.push_back(rule);

    // Rule for heredoc-style blocks (e.g., <<PYTHON ... PYTHON)
    // This is a simplified rule; a real implementation would need state across blocks.
    rule.pattern = std::regex("<<[A-Z_]+");
    rule.format_name = "heredoc_marker";
    highlighting_rules.push_back(rule);
}

void TissSyntaxHighlighter::highlightBlock(const std::string& text) {
    // This function would be called by the GUI framework for each line/block of text.
    // It iterates through the rules and applies formatting to matching text.
    std::cout << "TissSyntaxHighlighter: Highlighting block: \"" << text << "\"" << std::endl;

    for (const auto& rule : highlighting_rules) {
        auto words_begin = std::sregex_iterator(text.begin(), text.end(), rule.pattern);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            std::string match_str = match.str();
            // In a real app, we would apply a format (e.g., color, bold) to the
            // matched range in the document.
            std::cout << "  - Found match for format '" << rule.format_name
                      << "': " << match_str << " at position " << match.position() << std::endl;
        }
    }
}
