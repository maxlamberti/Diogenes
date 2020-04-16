#ifndef LOADINGSCREENDIALOG_H
#define LOADINGSCREENDIALOG_H

#include <QDialog>

namespace Ui {
class LoadingScreenDialog;
}

class LoadingScreenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingScreenDialog(QWidget *parent = nullptr);
    ~LoadingScreenDialog();
    void UpdateLoadingScreenText(std::string text);

private:
    Ui::LoadingScreenDialog *ui;
};

#endif // LOADINGSCREENDIALOG_H
