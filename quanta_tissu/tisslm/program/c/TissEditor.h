#ifndef TISSEDITOR_H
#define TISSEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>

class TissEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    TissEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *line_number_area;
};

#endif // TISSEDITOR_H
