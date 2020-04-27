#include <QApplication>
#include <aws/core/Aws.h>

#include "mainwindow.h"


const QString APP_NAME("Diogenes");

int main(int argc, char *argv[])
{
    // Start Application
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    QCoreApplication::setApplicationName(APP_NAME);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(APP_NAME);
    w.show();
    int exec_code = a.exec();
    Aws::ShutdownAPI(options);

    return exec_code;
}
