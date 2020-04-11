#include <QApplication>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    // Start Application
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
