#ifndef TISSLINTER_H
#define TISSLINTER_H

#include <QString>
#include <QList>
#include <QMap>

class TissLinter
{
public:
    TissLinter();

    // The main linting function.
    // Returns a map where the key is the line number and the value is a list of error strings.
    QMap<int, QList<QString>> lint(const QString &text);
};

#endif // TISSLINTER_H
