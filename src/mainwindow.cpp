#include <map>
#include <string>
#include <iostream>
#include <algorithm>
#include <aws/core/Aws.h>

#include "awsutils.hpp"
#include "mainwindow.h"
#include "./ui_mainwindow.h"


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

    std::cout << "Launching instance" << std::endl;
    this->aws_utils.launchSpotInstance();

}

void MainWindow::comboboxItemChanged(QString selection){

//    auto reverse_instance_type_mapper = Aws::EC2::Model::InstanceTypeMapper::GetInstanceTypeForName;
    this->aws_utils.notebookConfig.instanceType = Aws::EC2::Model::InstanceTypeMapper::GetInstanceTypeForName(selection.toStdString().c_str());

}

MainWindow::~MainWindow()
{
    delete ui;
}

