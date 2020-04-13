#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>
#include <string>
#include <QMainWindow>

#include "awsutils.hpp"
#include "launchdialog.h"
#include "loadingscreendialog.h"
#include <selectregiondialog.h>

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

private slots:
    void LaunchButtonPressed();
    void InstanceTypeChanged(QString selection);
    void PopulateInstanceTypeSelection();
    void SetRegion();

};
#endif // MAINWINDOW_H
