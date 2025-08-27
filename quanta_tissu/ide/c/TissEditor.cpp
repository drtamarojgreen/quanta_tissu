#include "TissEditor.h"
#include <iostream>

TissEditor::TissEditor() : MockTextEditWidget() {
    // Constructor for the TissEditor.
    std::cout << "TissEditor: Constructor called." << std::endl;

    // In a real editor, we would create and configure the line number area here.
    // lineNumberArea = new LineNumberArea(this);
    std::cout << "TissEditor: Line number area would be created here." << std::endl;

    // We might also set up connections for signals and slots, for example:
    // connect(this, &TissEditor::blockCountChanged, this, &TissEditor::updateLineNumberAreaWidth);
}

TissEditor::~TissEditor() {
    std::cout << "TissEditor: Destructor called." << std::endl;
}

void TissEditor::setLineNumberAreaWidth(int width) {
    // This function would be called to update the width of the line number area
    // as the number of lines in the document changes.
    std::cout << "TissEditor: Setting line number area width to " << width << " pixels." << std::endl;
}
