#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>
#include <string>
#include <QMainWindow>

#include "awsutils.hpp"
#include "launchdialog.h"
#include "loadingscreendialog.h"

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
    Ui::LaunchDialog *launch_dialog_ui;
    Ui::LoadingScreenDialog *loading_screen_ui;


private slots:
      void LaunchButtonPressed();
      void ComboboxItemChanged(QString selection);

};
#endif // MAINWINDOW_H
