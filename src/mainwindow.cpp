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

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        AwsUtils aws_utils;
        auto all_instance_types = aws_utils.GetSpotInstanceTypes();
//        print_vector(all_instance_types);

        ui->setupUi(this);

        ui->comboBox->setStyleSheet("combobox-popup: 0;");

        connect(ui->pushButton, SIGNAL(released()), this, SLOT(launch_button_pressed()));

        for (auto val : all_instance_types) {
            QString s(val.c_str());
            ui->comboBox->addItem(s);
        }
    }
    Aws::ShutdownAPI(options);
}

void MainWindow::launch_button_pressed() {

    std::cout << "Launching instance" << std::endl;

}

MainWindow::~MainWindow()
{
    delete ui;
}

