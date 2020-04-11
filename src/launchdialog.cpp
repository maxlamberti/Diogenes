#include <QUrl>
#include <QtCore>
#include <QDesktopServices>

#include "launchdialog.h"
#include "ui_launchdialog.h"

LaunchDialog::LaunchDialog(QWidget *parent, AwsUtils *aws_utils) :
    QDialog(parent),
    ui(new Ui::LaunchDialog),
    aws_utils(aws_utils) {
    ui->setupUi(this);

    connect(ui->LaunchJupyterLabButton, SIGNAL(released()), this, SLOT(LaunchJupyterButtonPressed()));
    connect(ui->TerminateInstanceButton, SIGNAL(released()), this, SLOT(TerminationButtonPressed()));
    connect(ui->RefreshConnectionButton, SIGNAL(released()), this, SLOT(RefreshConnectionButtonPressed()));

}

LaunchDialog::~LaunchDialog() {
    delete ui;
}

void LaunchDialog::UpdateLabelWithNotebookInfo(NotebookConfig notebook_data) {
    std::string dialog_text = "Price per hour: " + notebook_data.price + "$<br><br>";
    dialog_text += "Instance ID: " + notebook_data.instanceId + "<br><br>";
    dialog_text += "Public IP: " + notebook_data.publicIp + "<br><br>";
    this->ui->InfoText->setWordWrap(true);
    this->ui->InfoText->setText(QString(dialog_text.c_str()));
    this->ui->InfoText->setTextFormat(Qt::RichText);
    this->ui->InfoText->setTextInteractionFlags(Qt::TextBrowserInteraction);
    this->ui->InfoText->setOpenExternalLinks(true);
}

void LaunchDialog::UpdateLabel(std::string text) {
    this->ui->InfoText->setWordWrap(true);
    this->ui->InfoText->setText(QString(text.c_str()));
    this->ui->InfoText->setTextFormat(Qt::RichText);
}

void LaunchDialog::LaunchJupyterButtonPressed() {
    QDesktopServices::openUrl(QUrl(this->aws_utils->notebookConfig.notebookUrl.c_str(), QUrl::TolerantMode));
}

void LaunchDialog::TerminationButtonPressed() {
    bool success = this->aws_utils->TerminateInstance();
    this->aws_utils->DeleteSecurityGroup();
    this->aws_utils->ResetConfigParameters();
    this->accept();  // TODO: open warning window if termination not successful
}

void LaunchDialog::RefreshConnectionButtonPressed() {
    this->aws_utils->RefreshConnection();
}
