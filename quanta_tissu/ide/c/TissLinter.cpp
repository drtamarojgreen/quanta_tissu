#include "TissLinter.h"
#include <QStringList>
#include <QRegularExpression>

TissLinter::TissLinter()
{
    // Constructor
}

QMap<int, QList<QString>> TissLinter::lint(const QString &text)
{
    QMap<int, QList<QString>> errors;
    QStringList lines = text.split('\n');

    bool in_task_block = false;
    int task_indent_level = -1;

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];
        int line_num = i + 1;
        QString stripped_line = line.trimmed();
        int current_indent = line.length() - line.ltrimmed().length();

        if (stripped_line.isEmpty() || stripped_line.startsWith("#")) {
            continue;
        }

        QList<QString> line_errors;

        // Rule: Check indentation and block context
        if (stripped_line.startsWith("TASK")) {
            in_task_block = true;
            task_indent_level = current_indent;
        } else if (in_task_block && current_indent <= task_indent_level) {
            in_task_block = false;
            task_indent_level = -1;
        }

        // Rule: STEP should be inside a TASK
        if (stripped_line.startsWith("STEP") && !in_task_block) {
            line_errors.append("Warning: STEP command should be inside a TASK block.");
        }

        // Rule: WRITE should be followed by a string or heredoc
        if (stripped_line.startsWith("WRITE")) {
            QString rest_of_line = stripped_line.mid(5).trimmed();
            if (!rest_of_line.startsWith('"') && !rest_of_line.startsWith("<<")) {
                line_errors.append("Warning: WRITE command should be followed by a string or heredoc.");
            }
        }

        // Rule: ASSERT should have an expression
        if (stripped_line.startsWith("ASSERT") && stripped_line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size() < 2) {
            line_errors.append("Warning: ASSERT command is missing an expression.");
        }

        if (!line_errors.isEmpty()) {
            errors[line_num] = line_errors;
        }
    }

    return errors;
}
