#include <iostream>
#include <vector>
#include <string>

// Qt headers for data types and file I/O
#include <QString>
#include <QList>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication> // For argument handling

// Include the TissLinter class from the parent directory
#include "../TissLinter.h"

void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options] <file1> <file2> ...\n\n"
              << "A command-line tool to lint .tiss files.\n\n"
              << "Options:\n"
              << "  -h, --help    Show this help message and exit\n"
              << std::endl;
}

int main(int argc, char *argv[]) {
    // A mock QApplication is needed for some Qt features, even in a console app
    QCoreApplication app(argc, argv);
    QStringList args = QCoreApplication::arguments();

    if (args.contains("-h") || args.contains("--help")) {
        print_usage(argv[0]);
        return 0;
    }

    // Filter out the program name and any options to get the list of files
    QList<QString> files_to_lint;
    for (int i = 1; i < args.size(); ++i) {
        if (!args.at(i).startsWith("-")) {
            files_to_lint.append(args.at(i));
        }
    }

    if (files_to_lint.isEmpty()) {
        std::cerr << "Error: No input files specified." << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    TissLinter linter;
    int total_errors = 0;

    for (const QString& filepath : files_to_lint) {
        QFile file(filepath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            std::cerr << "Error: Cannot open file '" << filepath.toStdString() << "'." << std::endl;
            total_errors++;
            continue;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        if (content.isEmpty() && file.size() > 0) {
             std::cerr << "Error: Failed to read content from '" << filepath.toStdString() << "'." << std::endl;
             total_errors++;
             continue;
        }

        QMap<int, QList<QString>> errors = linter.lint(content);

        if (!errors.isEmpty()) {
            std::cout << "Errors found in: " << filepath.toStdString() << std::endl;
            QMapIterator<int, QList<QString>> i(errors);
            while (i.hasNext()) {
                i.next();
                int line_number = i.key();
                const QList<QString>& line_errors = i.value();
                for (const QString& error_msg : line_errors) {
                    std::cout << "  " << filepath.toStdString() << ":" << line_number
                              << ": " << error_msg.toStdString() << std::endl;
                    total_errors++;
                }
            }
        } else {
             std::cout << "No errors found in: " << filepath.toStdString() << std::endl;
        }
    }

    if (total_errors > 0) {
        std::cout << "\nLinting complete. Found " << total_errors << " error(s)." << std::endl;
        return 1; // Exit with error code if issues were found
    }

    std::cout << "\nLinting complete. No errors found." << std::endl;
    return 0;
}
