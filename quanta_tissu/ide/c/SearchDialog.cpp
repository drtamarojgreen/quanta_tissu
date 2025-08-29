#include "SearchDialog.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>

SearchDialog::SearchDialog(QWidget *parent)
    : QDialog(parent)
{
    // Create widgets
    search_line_edit = new QLineEdit;
    replace_line_edit = new QLineEdit;
    case_sensitive_checkbox = new QCheckBox(tr("Case Sensitive"));
    regex_checkbox = new QCheckBox(tr("Use Regular Expressions"));
    find_button = new QPushButton(tr("&Find Next"));
    replace_button = new QPushButton(tr("&Replace"));
    replace_all_button = new QPushButton(tr("Replace &All"));
    close_button = new QPushButton(tr("Close"));

    // Macro widgets
    macro_combo_box = new QComboBox;
    save_macro_button = new QPushButton(tr("Save Macro"));
    delete_macro_button = new QPushButton(tr("Delete Macro"));

    // Set up layout
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel(tr("Find:")), 0, 0);
    layout->addWidget(search_line_edit, 0, 1);
    layout->addWidget(find_button, 0, 2);
    layout->addWidget(new QLabel(tr("Replace:")), 1, 0);
    layout->addWidget(replace_line_edit, 1, 1);
    layout->addWidget(replace_button, 1, 2);
    layout->addWidget(case_sensitive_checkbox, 2, 0);
    layout->addWidget(regex_checkbox, 2, 1);
    layout->addWidget(replace_all_button, 2, 2);

    // Add a separator for macros
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line, 3, 0, 1, 3);

    layout->addWidget(new QLabel(tr("Macros:")), 4, 0);
    layout->addWidget(macro_combo_box, 4, 1);
    layout->addWidget(save_macro_button, 5, 1);
    layout->addWidget(delete_macro_button, 5, 2);

    layout->addWidget(close_button, 6, 2);
    setLayout(layout);

    setWindowTitle(tr("Find and Replace"));

    // Connect signals and slots
    connect(find_button, &QPushButton::clicked, this, &SearchDialog::findClicked);
    connect(replace_button, &QPushButton::clicked, this, &SearchDialog::replaceClicked);
    connect(replace_all_button, &QPushButton::clicked, this, &SearchDialog::replaceAllClicked);
    connect(close_button, &QPushButton::clicked, this, &QDialog::close);

    // Macro connections
    connect(save_macro_button, &QPushButton::clicked, this, &SearchDialog::saveMacro);
    connect(delete_macro_button, &QPushButton::clicked, this, &SearchDialog::deleteMacro);
    connect(macro_combo_box, QOverload<int>::of(&QComboBox::activated), this, &SearchDialog::macroSelected);

    loadMacros();
}

void SearchDialog::findClicked()
{
    Qt::CaseSensitivity cs = case_sensitive_checkbox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool use_regex = regex_checkbox->isChecked();
    emit findNext(search_line_edit->text(), cs, use_regex);
}

void SearchDialog::replaceClicked()
{
    Qt::CaseSensitivity cs = case_sensitive_checkbox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool use_regex = regex_checkbox->isChecked();
    emit replace(search_line_edit->text(), replace_line_edit->text(), cs, use_regex);
}

void SearchDialog::replaceAllClicked()
{
    Qt::CaseSensitivity cs = case_sensitive_checkbox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool use_regex = regex_checkbox->isChecked();
    emit replaceAll(search_line_edit->text(), replace_line_edit->text(), cs, use_regex);
}

void SearchDialog::loadMacros()
{
    QSettings settings;
    settings.beginGroup("SearchMacros");
    macro_combo_box->clear();
    macro_combo_box->addItems(settings.childKeys());
    settings.endGroup();
}

void SearchDialog::saveMacro()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Save Macro"),
                                         tr("Macro name:"), QLineEdit::Normal,
                                         macro_combo_box->currentText(), &ok);
    if (ok && !name.isEmpty()) {
        QSettings settings;
        settings.beginGroup("SearchMacros");

        if (settings.contains(name)) {
            if (QMessageBox::question(this, tr("Overwrite Macro"),
                                      tr("A macro named '%1' already exists. Do you want to overwrite it?").arg(name),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
                settings.endGroup();
                return;
            }
        }

        settings.setValue(name, QStringList() << search_line_edit->text() << replace_line_edit->text());
        settings.endGroup();
        loadMacros();
        macro_combo_box->setCurrentText(name);
    }
}

void SearchDialog::deleteMacro()
{
    QString name = macro_combo_box->currentText();
    if (name.isEmpty())
        return;

    if (QMessageBox::question(this, tr("Delete Macro"), tr("Are you sure you want to delete the macro '%1'?").arg(name),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QSettings settings;
        settings.beginGroup("SearchMacros");
        settings.remove(name);
        settings.endGroup();
        loadMacros();
    }
}

void SearchDialog::macroSelected(int index)
{
    QString name = macro_combo_box->itemText(index);
    if (name.isEmpty())
        return;

    QSettings settings;
    settings.beginGroup("SearchMacros");
    QStringList macro = settings.value(name).toStringList();
    settings.endGroup();

    if (macro.size() == 2) {
        search_line_edit->setText(macro[0]);
        replace_line_edit->setText(macro[1]);
    }
}
