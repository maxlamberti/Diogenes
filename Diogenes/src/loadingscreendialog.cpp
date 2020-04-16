#include "loadingscreendialog.h"
#include "ui_loadingscreendialog.h"

LoadingScreenDialog::LoadingScreenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadingScreenDialog)
{
    ui->setupUi(this);
}

LoadingScreenDialog::~LoadingScreenDialog()
{
    delete ui;
}

void LoadingScreenDialog::UpdateLoadingScreenText(std::string text) {
    this->ui->LoadingScreenText->setWordWrap(true);
    this->ui->LoadingScreenText->setText(QString(text.c_str()));
    this->ui->LoadingScreenText->setTextFormat(Qt::RichText);
}
