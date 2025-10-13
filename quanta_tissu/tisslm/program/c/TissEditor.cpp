#include "TissEditor.h"
#include <QPainter>
#include <QTextBlock>

// A helper widget to display line numbers
class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TissEditor *editor) : QWidget(editor), code_editor(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(code_editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        code_editor->lineNumberAreaPaintEvent(event);
    }

private:
    TissEditor *code_editor;
};


TissEditor::TissEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    line_number_area = new LineNumberArea(this);

    connect(this, &TissEditor::blockCountChanged, this, &TissEditor::updateLineNumberAreaWidth);
    connect(this, &TissEditor::updateRequest, this, &TissEditor::updateLineNumberArea);
    connect(this, &TissEditor::cursorPositionChanged, this, &TissEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int TissEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void TissEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TissEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        line_number_area->scroll(0, dy);
    else
        line_number_area->update(0, rect.y(), line_number_area->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void TissEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    line_number_area->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TissEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extra_selections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor line_color = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(line_color);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extra_selections.append(selection);
    }

    setExtraSelections(extra_selections);
}

void TissEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(line_number_area);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int block_number = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(block_number + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, line_number_area->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++block_number;
    }
}
