#ifndef LAUNCHDIALOG_H
#define LAUNCHDIALOG_H

#include <string>
#include <QDialog>

namespace Ui {
class LaunchDialog;
}

class LaunchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LaunchDialog(QWidget *parent = nullptr);
    ~LaunchDialog();
    void UpdateLabelWithNotebookInfo(std::string new_text);

private:
    Ui::LaunchDialog *ui;

};

#endif // LAUNCHDIALOG_H
