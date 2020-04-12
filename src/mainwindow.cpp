#include <iostream>
#include <algorithm>

#include "awsutils.hpp"
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "launchdialog.h"
#include "loadingscreendialog.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {

    AwsUtils aws_utils;
    auto all_instance_types = aws_utils.GetSpotInstanceTypes();

    ui->setupUi(this);

    ui->comboBox->setStyleSheet("combobox-popup: 0;");

    connect(ui->pushButton, SIGNAL(released()), this, SLOT(LaunchButtonPressed()));
    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(ComboboxItemChanged(QString)));

    for (auto val : all_instance_types) {
        QString s(val.c_str());
        ui->comboBox->addItem(s);
    }
}

void MainWindow::LaunchButtonPressed() {

    // Display a loading screen that locks main window
    this->loading_screen = new LoadingScreenDialog(this);
    this->loading_screen->setModal(true);
    this->loading_screen->setAttribute(Qt::WA_DeleteOnClose);
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

void MainWindow::ComboboxItemChanged(QString selection){

//    auto reverse_instance_type_mapper = Aws::EC2::Model::InstanceTypeMapper::GetInstanceTypeForName;
    this->aws_utils.notebookConfig.instanceType = Aws::EC2::Model::InstanceTypeMapper::GetInstanceTypeForName(selection.toStdString().c_str());

}

MainWindow::~MainWindow()
{
    delete ui;
}

