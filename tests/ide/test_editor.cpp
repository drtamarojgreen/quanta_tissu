#include "../db/test_framework.h"
#include "../../quanta_tissu/ide/c/TissEditor.h"

// Test fixture for TissEditor
class TissEditorTest {
public:
    TissEditorTest() {
        // In a real application, the editor would be a GUI widget.
        // Here, we simulate its behavior with a simple string.
    }

protected:
    TissEditor editor;
};

TEST_CASE(Editor_InitialState) {
    TissEditorTest fixture;
    ASSERT_TRUE(fixture.editor.toPlainText().empty());
}

TEST_CASE(Editor_InsertText) {
    TissEditorTest fixture;
    fixture.editor.insertPlainText("Hello, world!");
    ASSERT_EQ("Hello, world!", fixture.editor.toPlainText());

    fixture.editor.insertPlainText(" More text.");
    ASSERT_EQ("Hello, world! More text.", fixture.editor.toPlainText());
}

TEST_CASE(Editor_SetText) {
    TissEditorTest fixture;
    fixture.editor.setPlainText("This is the new content.");
    ASSERT_EQ("This is the new content.", fixture.editor.toPlainText());

    fixture.editor.setPlainText("Overwritten.");
    ASSERT_EQ("Overwritten.", fixture.editor.toPlainText());
}

TEST_CASE(Editor_ClearText) {
    TissEditorTest fixture;
    fixture.editor.setPlainText("Some text to be cleared.");
    fixture.editor.clear();
    ASSERT_TRUE(fixture.editor.toPlainText().empty());
}

TEST_CASE(Editor_SearchText_Found) {
    TissEditorTest fixture;
    fixture.editor.setPlainText("One two three, one two three.");

    // Assume find() returns true if found and selects the text.
    ASSERT_TRUE(fixture.editor.find("two"));

    // Conceptually, check what text is "selected".
    // We'll assume a method `selectedText()` exists for testing.
    ASSERT_EQ("two", fixture.editor.selectedText());
}

TEST_CASE(Editor_SearchText_NotFound) {
    TissEditorTest fixture;
    fixture.editor.setPlainText("One two three.");

    ASSERT_FALSE(fixture.editor.find("four"));
    ASSERT_TRUE(fixture.editor.selectedText().empty());
}

TEST_CASE(Editor_UndoRedo) {
    TissEditorTest fixture;
    fixture.editor.setPlainText("Initial text.");
    fixture.editor.insertPlainText(" More text.");
    ASSERT_EQ("Initial text. More text.", fixture.editor.toPlainText());

    // Simulate undo
    fixture.editor.undo();
    ASSERT_EQ("Initial text.", fixture.editor.toPlainText());

    // Simulate redo
    fixture.editor.redo();
    ASSERT_EQ("Initial text. More text.", fixture.editor.toPlainText());
}
