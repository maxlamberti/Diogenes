#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <aws/core/Aws.h>
#include <aws/ec2/EC2Client.h>
#include <aws/ec2/EC2Request.h>
#include <aws/ec2/model/DescribeImagesRequest.h>
#include <aws/ec2/model/RequestSpotInstancesRequest.h>
#include <aws/ec2/model/DescribeInstanceTypesRequest.h>
#include <aws/ec2/model/DescribeSpotInstanceRequestsRequest.h>


template <typename T>
auto map_enum_vec_to_sorted_str_vec(std::vector<T> input_vector, Aws::String (*mapper)(T)) -> std::vector<Aws::String> {

  std::vector<Aws::String> result;
  result.reserve(input_vector.size());

  // map and sort values
  for (auto val : input_vector) {
    result.push_back(mapper(val));
  }
  std::sort(result.begin(), result.end());

  return result;
}

template <typename T>
void print_vector(std::vector<T> input_vector) {
  for (const auto& val : input_vector) {
    std::cout << val << std::endl;
  }
}


std::vector<Aws::String> get_spot_instance_types(Aws::EC2::EC2Client ec2_client) {

  // Get response from AWS
  Aws::EC2::Model::DescribeInstanceTypesRequest instance_type_request;
  auto instance_type_response = ec2_client.DescribeInstanceTypes(instance_type_request);
  auto instance_types_in_response = instance_type_response.GetResult().GetInstanceTypes();

  // Map response to string vector
  auto instance_type_mapper = Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType;
  std::vector<Aws::EC2::Model::InstanceType> instance_type_vec;
  instance_type_vec.reserve(instance_types_in_response.size());
  for (auto val : instance_types_in_response) {
    instance_type_vec.push_back(val.GetInstanceType());
  }
  std::vector<Aws::String> all_instance_types = map_enum_vec_to_sorted_str_vec(instance_type_vec, instance_type_mapper);

  return all_instance_types;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::Client::ClientConfiguration config;
        config.region = Aws::Region::US_EAST_2;
        Aws::EC2::EC2Client ec2_client(config);

        auto all_instance_types = get_spot_instance_types(ec2_client);
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

