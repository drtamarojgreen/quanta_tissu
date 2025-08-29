#include "../db/test_framework.h"
#include "../../quanta_tissu/ide/c/TissSyntaxHighlighter.h"

// Mock implementation for dependencies if needed
class MockTextDocument {
public:
    std::string toPlainText() const { return text; }
    void setText(const std::string& new_text) { text = new_text; }
private:
    std::string text;
};

// Test fixture for TissSyntaxHighlighter
class TissSyntaxHighlighterTest {
public:
    TissSyntaxHighlighterTest() : highlighter(&document) {}

protected:
    MockTextDocument document;
    TissSyntaxHighlighter highlighter;
};

TEST_CASE(SyntaxHighlighter_Keywords) {
    TissSyntaxHighlighterTest fixture;
    fixture.document.setText("SETUP { db = 'test.db' } ACTION");

    // This is a conceptual test.
    // In a real Qt application, we would check the formatting applied to the document.
    // Here, we'll assume the highlighter has a method to get matches for testing.
    auto highlights = fixture.highlighter.getHighlights("keyword");

    ASSERT_EQ(2, highlights.size());
    ASSERT_EQ("SETUP", highlights[0].text);
    ASSERT_EQ("ACTION", highlights[1].text);
}

TEST_CASE(SyntaxHighlighter_Comments) {
    TissSyntaxHighlighterTest fixture;
    fixture.document.setText("# This is a comment\nACTION");

    auto highlights = fixture.highlighter.getHighlights("comment");

    ASSERT_EQ(1, highlights.size());
    ASSERT_EQ("# This is a comment", highlights[0].text);
}

TEST_CASE(SyntaxHighlighter_Strings) {
    TissSyntaxHighlighterTest fixture;
    fixture.document.setText("ACTION { query = 'SELECT * FROM users' }");

    auto highlights = fixture.highlighter.getHighlights("string");

    ASSERT_EQ(1, highlights.size());
    ASSERT_EQ("'SELECT * FROM users'", highlights[0].text);
}

TEST_CASE(SyntaxHighlighter_MixedContent) {
    TissSyntaxHighlighterTest fixture;
    fixture.document.setText(
        "SETUP { db = 'main.db' } # Setup the database\n"
        "ACTION { query = 'get_users' } # Perform an action"
    );

    auto keyword_highlights = fixture.highlighter.getHighlights("keyword");
    ASSERT_EQ(2, keyword_highlights.size());
    ASSERT_EQ("SETUP", keyword_highlights[0].text);
    ASSERT_EQ("ACTION", keyword_highlights[1].text);

    auto string_highlights = fixture.highlighter.getHighlights("string");
    ASSERT_EQ(2, string_highlights.size());
    ASSERT_EQ("'main.db'", string_highlights[0].text);
    ASSERT_EQ("'get_users'", string_highlights[1].text);

    auto comment_highlights = fixture.highlighter.getHighlights("comment");
    ASSERT_EQ(2, comment_highlights.size());
    ASSERT_EQ("# Setup the database", comment_highlights[0].text);
    ASSERT_EQ("# Perform an action", comment_highlights[1].text);
}

TEST_CASE(SyntaxHighlighter_NoMatches) {
    TissSyntaxHighlighterTest fixture;
    fixture.document.setText("some random text with no keywords");

    ASSERT_TRUE(fixture.highlighter.getHighlights("keyword").empty());
    ASSERT_TRUE(fixture.highlighter.getHighlights("comment").empty());
    ASSERT_TRUE(fixture.highlighter.getHighlights("string").empty());
}
