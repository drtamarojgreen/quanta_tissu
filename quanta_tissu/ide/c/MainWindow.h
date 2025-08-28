#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// Forward declarations
class TissEditor;
class SearchDialog;
class QAction;
class QMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void documentWasModified();
    void showSearchDialog();

public slots:
    void findNext(const QString &str, Qt::CaseSensitivity cs, bool use_regex);
    void findPrevious(const QString &str, Qt::CaseSensitivity cs, bool use_regex);
    void replace(const QString &str, const QString &replace_str, Qt::CaseSensitivity cs, bool use_regex);
    void replaceAll(const QString &str, const QString &replace_str, Qt::CaseSensitivity cs, bool use_regex);

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    TissEditor *editor;
    SearchDialog *search_dialog;
    QString current_file;

    QMenu *file_menu;
    QMenu *edit_menu;
    QMenu *help_menu;
    QAction *new_action;
    QAction *open_action;
    QAction *save_action;
    QAction *save_as_action;
    QAction *exit_action;
    QAction *find_action;
    QAction *cut_action;
    QAction *copy_action;
    QAction *paste_action;
    QAction *about_action;
    QAction *about_qt_action;
};

#endif // MAINWINDOW_H
