#include "errordialog.hpp"
#include "ui_errordialog.h"

ErrorDialog::ErrorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
    connect(this->ui->CloseButton, SIGNAL(released()), this, SLOT(PressedCloseButton()));
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

void ErrorDialog::SetErrorMessage(std::string msg) {
    this->ui->ErrorMsgLabel->setText(msg.c_str());
}

void ErrorDialog::PressedCloseButton() {
    this->accept();
}
