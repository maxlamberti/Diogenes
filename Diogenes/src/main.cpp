#include <QApplication>
#include <aws/core/Aws.h>

#include "mainwindow.h"


const QString APP_NAME("Diogenes");

int main(int argc, char *argv[])
{
    // Start Application
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(APP_NAME);
    MainWindow w;
    w.setWindowTitle(QCoreApplication::applicationName());
    w.show();
    int exec_code = a.exec();
    Aws::ShutdownAPI(options);

    return exec_code;
}
