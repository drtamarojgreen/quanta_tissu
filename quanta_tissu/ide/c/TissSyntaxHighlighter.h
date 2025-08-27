#ifndef TISSSYNTAXHIGHLIGHTER_H
#define TISSSYNTAXHIGHLIGHTER_H

#include <iostream>
#include <string>
#include <vector>
#include <regex>

// Mock base class for a syntax highlighter. In Qt, this would be QSyntaxHighlighter.
// It typically operates on a document or a block of text.
class MockSyntaxHighlighter {
public:
    MockSyntaxHighlighter() {
        std::cout << "MockSyntaxHighlighter: Constructor." << std::endl;
    }
    virtual ~MockSyntaxHighlighter() {}

protected:
    // This virtual function is the core of the highlighter.
    // A real framework would call this for each block of text that needs highlighting.
    virtual void highlightBlock(const std::string& text) = 0;
};

// Defines the highlighting rules for TissLang.
class TissSyntaxHighlighter : public MockSyntaxHighlighter {
public:
    TissSyntaxHighlighter();
    ~TissSyntaxHighlighter();

protected:
    // The main highlighting function.
    void highlightBlock(const std::string& text) override;

private:
    // Represents a single highlighting rule (a regex pattern and its format).
    struct HighlightingRule {
        std::regex pattern;
        std::string format_name; // e.g., "keyword", "comment", "string"
    };

    std::vector<HighlightingRule> highlighting_rules;

    // --- Rule Initialization ---
    void initializeRules();
};

#endif // TISSSYNTAXHIGHLIGHTER_H
