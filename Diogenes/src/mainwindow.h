#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>
#include <string>
#include <QMainWindow>

#include "awsutils.hpp"
#include "launchdialog.h"
#include "loadingscreendialog.h"
#include "selectregiondialog.h"
#include "errordialog.hpp"
#include "credentialsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    AwsUtils aws_utils;

private:
    Ui::MainWindow *ui;
    LaunchDialog *launch_dialog;
    LoadingScreenDialog *loading_screen;
    SelectRegionDialog *region_screen;
    ErrorDialog *error_screen;
    CredentialsDialog *credentials_screen;
    void OpenErrorDialog(const std::string& error);
    int GetStorageSize();
    void OpenCredentialsScreen();
    void OpenRegionScreen();

private slots:
    void LaunchButtonPressed();
    void InstanceTypeChanged(QString selection);
    void PopulateInstanceTypeSelection();
    void SetRegion();
    void SetRegionAndCredentials();

};
#endif // MAINWINDOW_H
