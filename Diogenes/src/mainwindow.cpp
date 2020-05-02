#include <iostream>
#include <algorithm>

#include "awsutils.hpp"
#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {

    AwsUtils aws_utils;
    this->ui->setupUi(this);
    this->aws_utils.notebookConfig.hasSystemCredentials ? this->OpenRegionScreen() : this->OpenCredentialsScreen();

    connect(this->ui->RequestInstanceButton, SIGNAL(released()), this, SLOT(LaunchButtonPressed()));
    connect(this->ui->InstanceTypeComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(InstanceTypeChanged(QString)));

    this->ui->InstanceTypeComboBox->addItem(QString("Select Instance Type"));
    this->ui->InstanceTypeComboBox->setStyleSheet("combobox-popup: 0;");
    this->ui->VolumeSizeLineEdit->setPlaceholderText(QString("Storage Size (8GB Default)"));

}

void MainWindow::OpenCredentialsScreen() {

    // Ask for credentials if none are found, also queries for region
    this->credentials_screen = new CredentialsDialog(this, aws_utils.AvailableRegions);
    this->credentials_screen->setAttribute(Qt::WA_DeleteOnClose);
    this->credentials_screen->setModal(true);
    connect(this->credentials_screen, SIGNAL(DataIsSet()),this, SLOT(SetRegionAndCredentials()));
    this->credentials_screen->open();
}

void MainWindow::OpenRegionScreen() {

    // Display screen querying for region
    this->region_screen = new SelectRegionDialog(this, aws_utils.AvailableRegions);
    this->region_screen->setAttribute(Qt::WA_DeleteOnClose);
    this->region_screen->setModal(true);
    connect(this->region_screen, SIGNAL(RegionIsSet()),this, SLOT(SetRegion()));
    this->region_screen->open();
}

void MainWindow::PopulateInstanceTypeSelection() {
    auto all_instance_types = this->aws_utils.GetSpotInstanceTypes();
    for (auto inst_type : all_instance_types) {
        this->ui->InstanceTypeComboBox->addItem(QString(inst_type.c_str()));
    }
}

void MainWindow::SetRegion() {
    this->aws_utils.notebookConfig.region = this->region_screen->SelectedRegion;
    this->PopulateInstanceTypeSelection();
}

void MainWindow::SetRegionAndCredentials() {
    this->aws_utils.SetCredentials(
        this->credentials_screen->AccessKeyId,
        this->credentials_screen->SecretKey
        );
    this->aws_utils.notebookConfig.region = this->credentials_screen->SelectedRegion;
    this->PopulateInstanceTypeSelection();
}

void MainWindow::OpenErrorDialog(const std::string& error) {
    this->error_screen = new ErrorDialog(this);
    this->error_screen->setAttribute(Qt::WA_DeleteOnClose);
    this->error_screen->setModal(true);
    this->error_screen->SetErrorMessage(error);
    this->error_screen->open();
}

int MainWindow::GetStorageSize() {
    int storage_size = 0;
    auto input_text = this->ui->VolumeSizeLineEdit->text().toStdString();
    if (input_text.size() > 0) {
        try {
            storage_size = std::stoi(input_text);
        }
        catch (const std::exception &e) {
            storage_size = 0;
        }
    }
    return storage_size;
}

void MainWindow::LaunchButtonPressed() {  // TODO: open error screen if instance type not selected

    // Get and sanitize storage volume input
    this->aws_utils.notebookConfig.blockSize = this->GetStorageSize();
    this->aws_utils.notebookConfig.deleteStorage = this->ui->DeleteStorageButton->isChecked();

    // Display a loading screen that locks main window
    this->loading_screen = new LoadingScreenDialog(this);
    this->loading_screen->setAttribute(Qt::WA_DeleteOnClose);
    this->loading_screen->setModal(true);
    this->loading_screen->UpdateLoadingScreenText("Launching instance... <br><br><br>May take a few minutes...");
    this->loading_screen->open();

    // Launch spot instance
    this->aws_utils.CreateKeyPair();
    this->aws_utils.CreateSecurityGroup();
    bool launch_was_success = false;
    try {
        this->aws_utils.LaunchSpotInstance();
        launch_was_success = true;
    } catch (const std::runtime_error& error) {
        this->loading_screen->accept();
        this->OpenErrorDialog(error.what());
    }

    // Open notebook launch dialog
    if (launch_was_success) {
        this->loading_screen->accept();
        this->launch_dialog = new LaunchDialog(this, &this->aws_utils);
        this->launch_dialog->setModal(true);
        this->launch_dialog->setAttribute(Qt::WA_DeleteOnClose);
        this->launch_dialog->UpdateLabelWithNotebookInfo(this->aws_utils.notebookConfig);
        this->launch_dialog->open();
    }
}

void MainWindow::InstanceTypeChanged(QString selection){

    // Check if valid type and set instance type
    auto min_type = Aws::EC2::Model::InstanceType::t1_micro;
    auto max_type = Aws::EC2::Model::InstanceType::inf1_24xlarge;
    auto selected_type = Aws::EC2::Model::InstanceTypeMapper::GetInstanceTypeForName(selection.toStdString().c_str());
    bool is_valid_type = (min_type <= selected_type) && (selected_type <= max_type);
    if (is_valid_type) {
        this->aws_utils.notebookConfig.instanceType = selected_type;
    }
}

MainWindow::~MainWindow() {
    delete ui;
}
