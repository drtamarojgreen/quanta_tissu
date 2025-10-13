#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("TissCo");
    QCoreApplication::setApplicationName("TissLang IDE");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    MainWindow main_window;
    main_window.show();

    return app.exec();
}
