#ifndef ERRORDIALOG_HPP
#define ERRORDIALOG_HPP

#include <QDialog>

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent = nullptr);
    ~ErrorDialog();
    void SetErrorMessage(std::string msg);

private:
    Ui::ErrorDialog *ui;

private slots:
    void PressedCloseButton();
};

#endif // ERRORDIALOG_HPP
