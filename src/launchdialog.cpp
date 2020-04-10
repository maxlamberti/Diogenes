#include "launchdialog.h"
#include "ui_launchdialog.h"

LaunchDialog::LaunchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LaunchDialog)
{
    ui->setupUi(this);
}

LaunchDialog::~LaunchDialog()
{
    delete ui;
}

void LaunchDialog::UpdateLabelWithNotebookInfo(std::string notebook_url) {
    notebook_url = "<a href=\"" + notebook_url + "\">Open Jupyter Lab</a>";
    this->ui->label->setText(QString(notebook_url.c_str()));
    this->ui->label->setTextFormat(Qt::RichText);
    this->ui->label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    this->ui->label->setOpenExternalLinks(true);
}
