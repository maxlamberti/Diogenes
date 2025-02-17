#ifndef LAUNCHDIALOG_H
#define LAUNCHDIALOG_H

#include <string>
#include <QDialog>

#include "awsutils.hpp"

namespace Ui {
class LaunchDialog;
}

class LaunchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LaunchDialog(QWidget *parent = nullptr, AwsUtils *aws_utils = nullptr);
    ~LaunchDialog();
    void UpdateLabelWithNotebookInfo(NotebookConfig notebook_data);
    void UpdateLabel(std::string text);
    AwsUtils *aws_utils;

private:
    Ui::LaunchDialog *ui;

private slots:
    void LaunchJupyterButtonPressed();
    void TerminationButtonPressed();
    void RefreshConnectionButtonPressed();

};

#endif // LAUNCHDIALOG_H
