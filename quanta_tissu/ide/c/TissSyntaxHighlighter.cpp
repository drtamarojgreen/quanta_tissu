#include "TissSyntaxHighlighter.h"

TissSyntaxHighlighter::TissSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keyword format
    keyword_format.setForeground(Qt::darkBlue);
    keyword_format.setFontWeight(QFont::Bold);
    const QString keyword_patterns[] = {
        QStringLiteral("\\b(TASK|STEP|SETUP|READ|WRITE|RUN|ASSERT|AS|EXIT_CODE|STDOUT|STDERR|FILE|EXISTS|IF|ELSE|DEFINE_TASK|TRY|CATCH|PAUSE|REQUEST_REVIEW|CHOOSE|OPTION|ESTIMATE_COST|SET_BUDGET|PROMPT_AGENT|INTO)\\b")
    };
    for (const QString &pattern : keyword_patterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keyword_format;
        highlighting_rules.append(rule);
    }

    // Directive format (matches Python IDE)
    directive_format.setForeground(Qt::magenta); // Magenta
    rule.pattern = QRegularExpression(QStringLiteral("@[a-zA-Z_]+"));
    rule.format = directive_format;
    highlighting_rules.append(rule);

    // Operator format (matches Python IDE)
    operator_format.setForeground(Qt::red); // Red
    rule.pattern = QRegularExpression(QStringLiteral("\\b(CONTAINS|IS_EMPTY|==)\\b"));
    rule.format = operator_format;
    highlighting_rules.append(rule);

    // Special Variable format (matches Python IDE)
    special_var_format.setForeground(Qt::cyan); // Cyan
    rule.pattern = QRegularExpression(QStringLiteral("\\b(LAST_RUN)\\b"));
    rule.format = special_var_format;
    highlighting_rules.append(rule);

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

    // Heredoc format (for the delimiter line itself)
    heredoc_format.setForeground(Qt::darkCyan);

    // Heredoc state expressions
    heredoc_start_expression = QRegularExpression(QStringLiteral("<<([A-Z_]+)"));
}

void TissSyntaxHighlighter::highlightBlock(const QString &text)
{
    // Apply single-line rules first
    for (const HighlightingRule &rule : highlighting_rules) {
        QRegularExpressionMatchIterator match_iterator = rule.pattern.globalMatch(text);
        while (match_iterator.hasNext()) {
            QRegularExpressionMatch match = match_iterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(NormalState);

    // Handle multi-line heredoc state
    if (previousBlockState() != InHeredoc) {
        QRegularExpressionMatch match = heredoc_start_expression.match(text);
        if (match.hasMatch()) {
            setCurrentBlockState(InHeredoc);
            // The delimiter is in capture group 1
            heredoc_end_expression.setPattern(QStringLiteral("^%1$").arg(match.captured(1)));
            setFormat(match.capturedStart(), match.capturedLength(), heredoc_format);
        }
    } else {
        QRegularExpressionMatch match = heredoc_end_expression.match(text);
        // If it's not the end of the heredoc, format the whole block
        if (!match.hasMatch()) {
            setCurrentBlockState(InHeredoc);
            setFormat(0, text.length(), heredoc_format);
        } else {
            // It's the end, so format up to the delimiter
            setFormat(0, match.capturedStart() + match.capturedLength(), heredoc_format);
            // The next block is in the normal state
            setCurrentBlockState(NormalState);
        }
    }
}
