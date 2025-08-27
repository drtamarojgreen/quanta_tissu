#include "TissSyntaxHighlighter.h"

TissSyntaxHighlighter::TissSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keyword format
    keyword_format.setForeground(Qt::darkBlue);
    keyword_format.setFontWeight(QFont::Bold);
    const QString keyword_patterns[] = {
        QStringLiteral("\\b(TASK|STEP|SETUP|READ|WRITE|RUN|ASSERT|AS|CONTAINS|IS_EMPTY|EXIT_CODE|LAST_RUN|STDOUT|STDERR|FILE|EXISTS|IF|ELSE|DEFINE_TASK|TRY|CATCH|PAUSE|REQUEST_REVIEW|CHOOSE|OPTION|ESTIMATE_COST|SET_BUDGET|PROMPT_AGENT|INTO)\\b")
    };
    for (const QString &pattern : keyword_patterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keyword_format;
        highlighting_rules.append(rule);
    }

    // Comment format
    comment_format.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = comment_format;
    highlighting_rules.append(rule);

    // String format
    string_format.setForeground(Qt::darkRed);
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = string_format;
    highlighting_rules.append(rule);

    // Pragma format
    pragma_format.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression(QStringLiteral("^#TISS!.*"));
    rule.format = pragma_format;
    highlighting_rules.append(rule);

    // Heredoc format
    heredoc_format.setForeground(Qt::darkCyan);
    rule.pattern = QRegularExpression(QStringLiteral("<<[A-Z_]+"));
    rule.format = heredoc_format;
    highlighting_rules.append(rule);
}

void TissSyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : highlighting_rules) {
        QRegularExpressionMatchIterator match_iterator = rule.pattern.globalMatch(text);
        while (match_iterator.hasNext()) {
            QRegularExpressionMatch match = match_iterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
