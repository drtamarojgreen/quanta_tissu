#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>

// Forward declarations
class QLineEdit;
class QCheckBox;
class QPushButton;
class QComboBox;

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);

signals:
    void findNext(const QString &str, Qt::CaseSensitivity cs, bool use_regex);
    void findPrevious(const QString &str, Qt::CaseSensitivity cs, bool use_regex);
    void replace(const QString &str, const QString &replace_str, Qt::CaseSensitivity cs, bool use_regex);
    void replaceAll(const QString &str, const QString &replace_str, Qt::CaseSensitivity cs, bool use_regex);

private slots:
    void findClicked();
    void replaceClicked();
    void replaceAllClicked();
    void saveMacro();
    void deleteMacro();
    void macroSelected(int index);

private:
    void loadMacros();

    QLineEdit *search_line_edit;
    QLineEdit *replace_line_edit;
    QCheckBox *case_sensitive_checkbox;
    QCheckBox *regex_checkbox;
    QPushButton *find_button;
    QPushButton *replace_button;
    QPushButton *replace_all_button;
    QPushButton *close_button;

    // Macro UI
    QComboBox *macro_combo_box;
    QPushButton *save_macro_button;
    QPushButton *delete_macro_button;
};

#endif // SEARCHDIALOG_H
