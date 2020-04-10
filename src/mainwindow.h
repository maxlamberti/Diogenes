#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>
#include <string>
#include <QMainWindow>

#include "awsutils.hpp"
#include "launchdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private:
    Ui::MainWindow *ui;
    AwsUtils aws_utils;
    LaunchDialog *launch_dialog;
    Ui::LaunchDialog *launch_dialog_ui;


private slots:
      void launchButtonPressed();
      void comboboxItemChanged(QString selection);

};
#endif // MAINWINDOW_H
