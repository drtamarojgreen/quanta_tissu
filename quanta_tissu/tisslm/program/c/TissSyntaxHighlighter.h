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

    enum BlockState {
        NormalState = -1,
        InHeredoc = 1
    };

    QRegularExpression heredoc_start_expression;
    QRegularExpression heredoc_end_expression;

    QTextCharFormat keyword_format;
    QTextCharFormat comment_format;
    QTextCharFormat string_format;
    QTextCharFormat pragma_format;
    QTextCharFormat heredoc_format;
    QTextCharFormat directive_format;
    QTextCharFormat operator_format;
    QTextCharFormat special_var_format;
};

#endif // TISSSYNTAXHIGHLIGHTER_H
