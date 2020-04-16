#include <QApplication>
#include "mainwindow.h"


const QString APP_NAME("Diogenes");

int main(int argc, char *argv[])
{
    // Start Application
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(APP_NAME);
    MainWindow w;
    w.setWindowTitle(QCoreApplication::applicationName());
    w.show();

    return a.exec();
}
