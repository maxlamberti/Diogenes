#include "credentialsdialog.h"
#include "ui_credentialsdialog.h"

#include <iostream>

CredentialsDialog::CredentialsDialog(QWidget *parent, std::set<std::string> AvailableRegions) :
    QDialog(parent),
    ui(new Ui::CredentialsDialog),
    AvailableRegions(AvailableRegions) {

    ui->setupUi(this);

    // Connect buttons
    connect(this->ui->ButtonBox, SIGNAL(accepted()), this, SLOT(OkButtonPressed()));
    connect(this->ui->ButtonBox, SIGNAL(rejected()), this, SLOT(CancelButtonPressed()));

    // Set up region combo box
    this->ui->RegionComboBox->addItem("Select Region");
    this->PopulateRegionComboBox();
}

void CredentialsDialog::PopulateRegionComboBox() {
    for (auto region : this->AvailableRegions) {
        this->ui->RegionComboBox->addItem(QString(region.c_str()));
    }
}

void CredentialsDialog::OkButtonPressed() {

    // Validate inputs
    std::string selected_region = this->ui->RegionComboBox->currentText().toStdString();
    bool is_valid_region = this->AvailableRegions.find(selected_region) != this->AvailableRegions.end();
    bool access_key_has_input = this->ui->AccessIdLine->text().size() > 0;
    bool secret_key_has_input = this->ui->SecretKeyIdLine->text().size() > 0;

    // Get inputs
    if (is_valid_region && access_key_has_input && secret_key_has_input) {
        this->SelectedRegion = selected_region;
        this->AccessKeyId = this->ui->AccessIdLine->text().toStdString();
        this->SecretKey = this->ui->SecretKeyIdLine->text().toStdString();
        std::cout << selected_region;
        emit this->DataIsSet();
        this->accept();
    }
}

void CredentialsDialog::CancelButtonPressed() {
    QApplication::quit();
}

CredentialsDialog::~CredentialsDialog() {
    delete ui;
}
