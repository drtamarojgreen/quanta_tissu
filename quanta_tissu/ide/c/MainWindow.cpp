#include "MainWindow.h"
#include "TissEditor.h"
#include "TissSyntaxHighlighter.h"
#include "SearchDialog.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QCloseEvent>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    editor = new TissEditor;
    setCentralWidget(editor);

    new TissSyntaxHighlighter(editor->document());
    search_dialog = new SearchDialog(this);

    createActions();
    createMenus();
    createStatusBar();

    readSettings();

    connect(editor->document(), &QTextDocument::contentsChanged,
            this, &MainWindow::documentWasModified);

    // Connect search dialog signals to main window slots
    connect(search_dialog, &SearchDialog::findNext, this, &MainWindow::findNext);
    connect(search_dialog, &SearchDialog::findPrevious, this, &MainWindow::findPrevious);
    connect(search_dialog, &SearchDialog::replace, this, &MainWindow::replace);
    connect(search_dialog, &SearchDialog::replaceAll, this, &MainWindow::replaceAll);

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        editor->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                   tr("Open Tiss File"), "",
                                   tr("Tiss Files (*.tiss);;All Files (*)"));
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::save()
{
    if (current_file.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(current_file);
    }
}

bool MainWindow::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About TissLang IDE"),
            tr("A conceptual IDE for the TissLang language."));
}

void MainWindow::documentWasModified()
{
    setWindowModified(editor->document()->isModified());
}

void MainWindow::showSearchDialog()
{
    search_dialog->show();
    search_dialog->raise();
    search_dialog->activateWindow();
}

void MainWindow::createActions()
{
    new_action = new QAction(tr("&New"), this);
    new_action->setShortcuts(QKeySequence::New);
    new_action->setStatusTip(tr("Create a new file"));
    connect(new_action, &QAction::triggered, this, &MainWindow::newFile);

    open_action = new QAction(tr("&Open..."), this);
    open_action->setShortcuts(QKeySequence::Open);
    open_action->setStatusTip(tr("Open an existing file"));
    connect(open_action, &QAction::triggered, this, &MainWindow::open);

    save_action = new QAction(tr("&Save"), this);
    save_action->setShortcuts(QKeySequence::Save);
    save_action->setStatusTip(tr("Save the document to disk"));
    connect(save_action, &QAction::triggered, this, &MainWindow::save);

    save_as_action = new QAction(tr("Save &As..."), this);
    save_as_action->setShortcuts(QKeySequence::SaveAs);
    save_as_action->setStatusTip(tr("Save the document under a new name"));
    connect(save_as_action, &QAction::triggered, this, &MainWindow::saveAs);

    exit_action = new QAction(tr("E&xit"), this);
    exit_action->setShortcuts(QKeySequence::Quit);
    exit_action->setStatusTip(tr("Exit the application"));
    connect(exit_action, &QAction::triggered, this, &QWidget::close);

    find_action = new QAction(tr("&Find/Replace..."), this);
    find_action->setShortcuts(QKeySequence::Find);
    find_action->setStatusTip(tr("Show the find and replace dialog"));
    connect(find_action, &QAction::triggered, this, &MainWindow::showSearchDialog);

    cut_action = new QAction(tr("Cu&t"), this);
    cut_action->setShortcuts(QKeySequence::Cut);
    cut_action->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(cut_action, &QAction::triggered, editor, &QPlainTextEdit::cut);

    copy_action = new QAction(tr("&Copy"), this);
    copy_action->setShortcuts(QKeySequence::Copy);
    copy_action->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(copy_action, &QAction::triggered, editor, &QPlainTextEdit::copy);

    paste_action = new QAction(tr("&Paste"), this);
    paste_action->setShortcuts(QKeySequence::Paste);
    paste_action->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(paste_action, &QAction::triggered, editor, &QPlainTextEdit::paste);

    about_action = new QAction(tr("&About"), this);
    about_action->setStatusTip(tr("Show the application's About box"));
    connect(about_action, &QAction::triggered, this, &MainWindow::about);

    about_qt_action = new QAction(tr("About &Qt"), this);
    about_qt_action->setStatusTip(tr("Show the Qt library's About box"));
    connect(about_qt_action, &QAction::triggered, qApp, &QApplication::aboutQt);

    cut_action->setEnabled(false);
    copy_action->setEnabled(false);
    connect(editor, &QPlainTextEdit::copyAvailable, cut_action, &QAction::setEnabled);
    connect(editor, &QPlainTextEdit::copyAvailable, copy_action, &QAction::setEnabled);
}

void MainWindow::createMenus()
{
    file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(new_action);
    file_menu->addAction(open_action);
    file_menu->addAction(save_action);
    file_menu->addAction(save_as_action);
    file_menu->addSeparator();
    file_menu->addAction(exit_action);

    edit_menu = menuBar()->addMenu(tr("&Edit"));
    edit_menu->addAction(find_action);
    edit_menu->addSeparator();
    edit_menu->addAction(cut_action);
    edit_menu->addAction(copy_action);
    edit_menu->addAction(paste_action);

    menuBar()->addSeparator();

    help_menu = menuBar()->addMenu(tr("&Help"));
    help_menu->addAction(about_action);
    help_menu->addAction(about_qt_action);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

bool MainWindow::maybeSave()
{
    if (!editor->document()->isModified())
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                                 tr("Cannot read file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    editor->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << editor->toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    current_file = fileName;
    editor->document()->setModified(false);
    setWindowModified(false);

    QString shownName = current_file;
    if (current_file.isEmpty())
        shownName = "untitled.tiss";
    setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::findNext(const QString &str, Qt::CaseSensitivity cs, bool use_regex)
{
    QTextDocument::FindFlags flags;
    if (cs == Qt::CaseInsensitive)
        flags |= QTextDocument::FindCaseSensitively;

    if (use_regex) {
        QRegularExpression regex(str, cs == Qt::CaseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
        if (!editor->find(regex, flags)) {
            statusBar()->showMessage(tr("Search string not found"), 2000);
        }
    } else {
        if (!editor->find(str, flags)) {
            statusBar()->showMessage(tr("Search string not found"), 2000);
        }
    }
}

void MainWindow::findPrevious(const QString &str, Qt::CaseSensitivity cs, bool use_regex)
{
    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    if (cs == Qt::CaseInsensitive)
        flags |= QTextDocument::FindCaseSensitively;

    if (use_regex) {
        QRegularExpression regex(str, cs == Qt::CaseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
        if (!editor->find(regex, flags)) {
            statusBar()->showMessage(tr("Search string not found"), 2000);
        }
    } else {
        if (!editor->find(str, flags)) {
            statusBar()->showMessage(tr("Search string not found"), 2000);
        }
    }
}

void MainWindow::replace(const QString &str, const QString &replace_str, Qt::CaseSensitivity cs, bool use_regex)
{
    if (editor->textCursor().hasSelection()) {
        QString selected_text = editor->textCursor().selectedText();
        bool match = use_regex ?
            selected_text.contains(QRegularExpression(str, cs == Qt::CaseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption)) :
            (selected_text.compare(str, cs) == 0);

        if (match) {
            editor->insertPlainText(replace_str);
        }
    }
    findNext(str, cs, use_regex);
}

void MainWindow::replaceAll(const QString &str, const QString &replace_str, Qt::CaseSensitivity cs, bool use_regex)
{
    int count = 0;
    editor->moveCursor(QTextCursor::Start);

    QTextDocument::FindFlags flags;
    if (cs == Qt::CaseInsensitive)
        flags |= QTextDocument::FindCaseSensitively;

    while(editor->find(str, flags)) {
        if (editor->textCursor().hasSelection()) {
            editor->insertPlainText(replace_str);
            count++;
        }
    }
    statusBar()->showMessage(tr("Replaced %1 occurrence(s)").arg(count), 2000);
}
