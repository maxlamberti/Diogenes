#include <map>
#include <string>
#include <iostream>
#include <algorithm>
#include <aws/core/Aws.h>

#include "awsutils.hpp"
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "launchdialog.h"


template <typename T>
void print_vector(std::vector<T> input_vector) {
  for (const auto& val : input_vector) {
    std::cout << val << std::endl;
  }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {

//    Aws::SDKOptions options;
//    Aws::InitAPI(options);

    AwsUtils aws_utils;
    auto all_instance_types = aws_utils.getSpotInstanceTypes();

    ui->setupUi(this);

    ui->comboBox->setStyleSheet("combobox-popup: 0;");

    connect(ui->pushButton, SIGNAL(released()), this, SLOT(launchButtonPressed()));
    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(comboboxItemChanged(QString)));

    for (auto val : all_instance_types) {
        QString s(val.c_str());
        ui->comboBox->addItem(s);
    }

//    Aws::ShutdownAPI(options);
}

void MainWindow::launchButtonPressed() {

    this->launch_dialog = new LaunchDialog(this);
    this->launch_dialog->setModal(true);
    this->launch_dialog->setAttribute(Qt::WA_DeleteOnClose);
    this->launch_dialog->show();
    std::string notebook_token_url = this->aws_utils.LaunchSpotInstance();
    this->launch_dialog->UpdateLabelWithNotebookInfo(notebook_token_url);

}

void MainWindow::comboboxItemChanged(QString selection){

//    auto reverse_instance_type_mapper = Aws::EC2::Model::InstanceTypeMapper::GetInstanceTypeForName;
    this->aws_utils.notebookConfig.instanceType = Aws::EC2::Model::InstanceTypeMapper::GetInstanceTypeForName(selection.toStdString().c_str());

}

MainWindow::~MainWindow()
{
    delete ui;
}

