//
// Created by Maximilien Lamberti on 4/7/20.
//

#include <algorithm>
#include <aws/core/Aws.h>
#include <aws/ec2/EC2Client.h>
#include <aws/ec2/EC2Request.h>
#include <aws/ec2/model/RequestSpotInstancesRequest.h>
#include <aws/ec2/model/DescribeInstanceTypesRequest.h>

#include "awsutils.hpp"


//std::map<std::string, std::string> DEFAULTS = {
//    {"instanceType", "c5.micro"},
//    {"imageId", "ami-0e01ce4ee18447327"},
//    {"keyName", "test-keys"},
//    {"region", "us-east-2"},
//
//};


AwsUtils::AwsUtils() {

  // Initialize EC2Client
  Aws::Client::ClientConfiguration client_config;
  client_config.region = Aws::Region::US_EAST_2;
  this->EC2Client = Aws::EC2::EC2Client(client_config);
  this->instanceType = "t2.micro";
    //    Aws::EC2::Model::InstanceType t2_micro_type = Aws::EC2::Model::InstanceType::t2_micro;
    this->notebookConfig.instanceType = Aws::EC2::Model::InstanceType::t2_micro;
  this->notebookConfig.keyName = "test-keys";
  this->notebookConfig.imageId = "ami-0e01ce4ee18447327";

//  this->notebookConfig = {
//      {"instanceType", "c5.micro"},
//      {"imageId", "ami-0e01ce4ee18447327"},
//      {"keyName", "test-keys"},
//      {"region", "us-east-2"},
//  };


}

AwsUtils::~AwsUtils() {
//  Aws::ShutdownAPI(this->SDKOptions);
}

template <typename T>
std::vector<Aws::String> AwsUtils::MapEnumVecToSortedStrVec(std::vector<T> input_vector, Aws::String (*mapper)(T)) {

  std::vector<Aws::String> result;
  result.reserve(input_vector.size());

  // map and sort values
  for (auto val : input_vector) {
    result.push_back(mapper(val));
  }
  std::sort(result.begin(), result.end());

  return result;

}

// instantiate template for specific data types
template std::vector<Aws::String> AwsUtils::MapEnumVecToSortedStrVec<Aws::EC2::Model::InstanceType>(std::vector<Aws::EC2::Model::InstanceType>,
                                                                                                    Aws::String (*)(Aws::EC2::Model::InstanceType));

auto AwsUtils::getSpotInstanceTypes() -> std::vector<Aws::String> {

  // Get response from AWS
  Aws::EC2::Model::DescribeInstanceTypesRequest instance_type_request;
  auto instance_type_response = this->EC2Client.DescribeInstanceTypes(instance_type_request);
  auto instance_types_in_response = instance_type_response.GetResult().GetInstanceTypes();

  // Map response to string vector
  auto instance_type_mapper = Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType;
  std::vector<Aws::EC2::Model::InstanceType> instance_type_vec;
  instance_type_vec.reserve(instance_types_in_response.size());
  for (auto val : instance_types_in_response) {
    instance_type_vec.push_back(val.GetInstanceType());
  }
  std::vector<Aws::String> all_instance_types = this->MapEnumVecToSortedStrVec(instance_type_vec, instance_type_mapper);

  return all_instance_types;

};


void AwsUtils::launchSpotInstance() {

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {

        Aws::EC2::Model::RequestSpotInstancesRequest spot_details;
        Aws::EC2::Model::RequestSpotLaunchSpecification launch_spec;
        Aws::Client::ClientConfiguration client_config;
        client_config.region = Aws::Region::US_EAST_2;
        Aws::EC2::EC2Client localEC2Client(client_config);

        launch_spec.SetInstanceType(this->notebookConfig.instanceType);
        launch_spec.SetKeyName(this->notebookConfig.keyName.c_str());
        launch_spec.SetImageId(this->notebookConfig.imageId.c_str());
        spot_details.SetInstanceCount(1);
        spot_details.SetLaunchSpecification(launch_spec);

        std::cout << "Init spot" << std::endl;

        auto response = localEC2Client.RequestSpotInstances(spot_details);
        std::cout << "Request is Success: " << response.IsSuccess() << std::endl;

    }
    Aws::ShutdownAPI(options);

//    auto err = response.GetError();
//    std::cout << err.GetMessage();
//    auto res = response.GetResult();
}
