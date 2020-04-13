#include <iostream>
#include <algorithm>

#include "awsutils.hpp"
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "launchdialog.h"
#include "loadingscreendialog.h"
#include <selectregiondialog.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {

    AwsUtils aws_utils;
    this->ui->setupUi(this);

    // Display a loading screen that locks main window
    this->region_screen = new SelectRegionDialog(this, aws_utils.AvailableRegions);
    this->region_screen->setAttribute(Qt::WA_DeleteOnClose);
    this->region_screen->setModal(true);
    this->region_screen->open();

    // Connect slots and signals: Order conveys priority
    connect(this->region_screen, SIGNAL(RegionIsSet()),this, SLOT(SetRegion()));  // PRIORITY: connected first
    connect(this->region_screen, SIGNAL(RegionIsSet()),this, SLOT(PopulateInstanceTypeSelection()));
    connect(this->ui->RequestInstanceButton, SIGNAL(released()), this, SLOT(LaunchButtonPressed()));
    connect(this->ui->InstanceTypeComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(InstanceTypeChanged(QString)));

    this->ui->InstanceTypeComboBox->addItem(QString("Select Instance Type"));
    this->ui->InstanceTypeComboBox->setStyleSheet("combobox-popup: 0;");

}

void MainWindow::PopulateInstanceTypeSelection() {
    auto all_instance_types = this->aws_utils.GetSpotInstanceTypes();
    for (auto inst_type : all_instance_types) {
        this->ui->InstanceTypeComboBox->addItem(QString(inst_type.c_str()));
    }
}

void MainWindow::SetRegion() {
    this->aws_utils.notebookConfig.region = this->region_screen->SelectedRegion;
}

void MainWindow::LaunchButtonPressed() {

    // Display a loading screen that locks main window
    this->loading_screen = new LoadingScreenDialog(this);
    this->loading_screen->setAttribute(Qt::WA_DeleteOnClose);
    this->loading_screen->setModal(true);
    this->loading_screen->UpdateLoadingScreenText("Launching instance... <br><br><br>May take a few minutes...");
    this->loading_screen->open();

    // Launch spot instance and open notebook dialog
    this->aws_utils.CreateKeyPair();
    this->aws_utils.CreateSecurityGroup();
    this->aws_utils.LaunchSpotInstance();
    this->loading_screen->accept();
    this->launch_dialog = new LaunchDialog(this, &this->aws_utils);
    this->launch_dialog->setModal(true);
    this->launch_dialog->setAttribute(Qt::WA_DeleteOnClose);
    this->launch_dialog->UpdateLabelWithNotebookInfo(this->aws_utils.notebookConfig);
    this->launch_dialog->open();
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
