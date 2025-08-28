#ifndef TISSSYNTAXHIGHLIGHTER_H
#define TISSSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class TissSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    TissSyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlighting_rules;

    QTextCharFormat keyword_format;
    QTextCharFormat comment_format;
    QTextCharFormat string_format;
    QTextCharFormat pragma_format;
    QTextCharFormat heredoc_format;
};

#endif // TISSSYNTAXHIGHLIGHTER_H
