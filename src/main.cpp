#include <QApplication>
#include <aws/core/Aws.h>

#include "mainwindow.h"


int main(int argc, char *argv[])
{

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    Aws::ShutdownAPI(options);

    return a.exec();
}
