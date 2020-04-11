#include <array>
#include <thread>
#include <chrono>
#include <cstdio>
#include <memory>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <filesystem>
#include <aws/core/Aws.h>
#include <aws/ec2/EC2Client.h>
#include <aws/ec2/EC2Request.h>
#include <aws/ec2/model/RequestSpotInstancesRequest.h>
#include <aws/ec2/model/DescribeInstanceTypesRequest.h>
#include <aws/ec2/model/DescribeInstanceStatusRequest.h>
#include <aws/ec2/model/DescribeInstancesRequest.h>
#include <aws/ec2/model/TerminateInstancesRequest.h>
#include <aws/ec2/model/CreateKeyPairRequest.h>
#include <aws/ec2/model/DeleteKeyPairRequest.h>

#include "awsutils.hpp"


std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}


AwsUtils::AwsUtils() {

  // Initialize EC2Client
  Aws::Client::ClientConfiguration client_config;
  client_config.region = Aws::Region::US_EAST_2;
  this->instanceType = "t2.micro";
  this->notebookConfig.instanceType = Aws::EC2::Model::InstanceType::t2_micro;
  this->notebookConfig.keyName = "DiogenesKey";
  this->notebookConfig.imageId = "ami-0e01ce4ee18447327";

}

AwsUtils::~AwsUtils() {}

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

    std::vector<Aws::String> all_instance_types;

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        // Set up client
        Aws::Client::ClientConfiguration client_config;
        client_config.region = Aws::Region::US_EAST_2;
        Aws::EC2::EC2Client ec2_client(client_config);

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
        all_instance_types = this->MapEnumVecToSortedStrVec(instance_type_vec, instance_type_mapper);
    }
    Aws::ShutdownAPI(options);

    return all_instance_types;

};

Aws::Vector<Aws::String> AwsUtils::CastToAwsStringVector(const std::string& str) {
    Aws::Vector<Aws::String> result_vec = {Aws::String(str)};
    return result_vec;
}

std::string AwsUtils::GetInstanceId(const Aws::Vector<Aws::String>& request_id, const Aws::EC2::EC2Client& ec2_client) {

    // Query for instance id
    Aws::EC2::Model::DescribeSpotInstanceRequestsRequest describe_spot_instance_request;
    describe_spot_instance_request.SetSpotInstanceRequestIds(request_id);
    auto describe_instance_response = ec2_client.DescribeSpotInstanceRequests(describe_spot_instance_request);

    std::string instance_id;
    auto instance_description_vec = describe_instance_response.GetResult().GetSpotInstanceRequests();
    if (instance_description_vec.size() != 0) {
        instance_id = instance_description_vec[0].GetInstanceId().c_str();
    }

    return instance_id;
};


Aws::EC2::Model::SummaryStatus AwsUtils::GetInstanceStatus(const std::string& instance_id, const Aws::EC2::EC2Client& ec2_client) {

    // Query for instance status
    Aws::EC2::Model::DescribeInstanceStatusRequest instance_status_request;
    auto aws_instance_id = this->CastToAwsStringVector(instance_id.c_str());
    instance_status_request.SetInstanceIds(aws_instance_id);
    auto instance_status_response = ec2_client.DescribeInstanceStatus(instance_status_request);

    // Sometimes response is empty
    auto instance_status_vec = instance_status_response.GetResult().GetInstanceStatuses();
    Aws::EC2::Model::SummaryStatus instance_status;
    std::cout << "Size of statuses: " << instance_status_vec.size() << "\nStatuses:" << std::endl;
    if (instance_status_vec.size() != 0) {
        for (auto val : instance_status_vec) {
            instance_status = val.GetInstanceStatus().GetStatus();
            std::cout << Aws::EC2::Model::SummaryStatusMapper::GetNameForSummaryStatus(instance_status).c_str() << std::endl;
        }
        instance_status = instance_status_vec[0].GetInstanceStatus().GetStatus();
    } else {
        instance_status = Aws::EC2::Model::SummaryStatus::NOT_SET;
    }

    return instance_status;
}

void AwsUtils::open_ssh_notebook_tunnel(std::string cmd) {
    exec(cmd.c_str());
}

void AwsUtils::LaunchSpotInstance() {

    std::string notebook_token_url;
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        // Set up client
        Aws::Client::ClientConfiguration client_config;
        client_config.region = Aws::Region::US_EAST_2;
        Aws::EC2::EC2Client ec2_client(client_config);

        // Request spot instance
        Aws::EC2::Model::RequestSpotInstancesRequest spot_details;
        Aws::EC2::Model::RequestSpotLaunchSpecification launch_spec;
        launch_spec.SetInstanceType(this->notebookConfig.instanceType);
        launch_spec.SetKeyName(this->notebookConfig.keyName.c_str());
        launch_spec.SetImageId(this->notebookConfig.imageId.c_str());
        spot_details.SetInstanceCount(1);
        spot_details.SetLaunchSpecification(launch_spec);
        auto response = ec2_client.RequestSpotInstances(spot_details);

        // Get initial data and request ID
        auto response_data = response.GetResult().GetSpotInstanceRequests();
        auto request_id = this->CastToAwsStringVector(response_data[0].GetSpotInstanceRequestId().c_str());
        auto price = response_data[0].GetSpotPrice();

        // Query instance status until it's ready to connect to
        std::string instance_id;
        Aws::EC2::Model::SummaryStatus instance_status;
        while (true) {

            if (instance_id.size() == 0) {
                instance_id = this->GetInstanceId(request_id, ec2_client);
            }

            std::this_thread::sleep_for(std::chrono::seconds (10));
            instance_status = this->GetInstanceStatus(instance_id, ec2_client);
            std::cout << (instance_status == Aws::EC2::Model::SummaryStatus::initializing) << std::endl;
            std::cout << "Current status: " << Aws::EC2::Model::SummaryStatusMapper::GetNameForSummaryStatus(instance_status).c_str() << std::endl;
            if (instance_status == Aws::EC2::Model::SummaryStatus::ok) {
                break;
            }
        }

        // Get public IP address of instance
        Aws::EC2::Model::Filter filter;
        Aws::EC2::Model::DescribeInstancesRequest describe_instance_request;
        filter.SetName("instance-id");
        filter.SetValues(this->CastToAwsStringVector(instance_id));
        describe_instance_request.AddFilters(filter);
        auto describe_instances_response = ec2_client.DescribeInstances(describe_instance_request);
        auto ip_address = describe_instances_response.GetResult().GetReservations()[0].GetInstances()[0].GetPublicIpAddress();
        std::string ip_address_str = ip_address.c_str();

        // Run install scripts
        std::string ssh_base = "ssh -oStrictHostKeyChecking=no -i " + this->notebookConfig.keyPath + " ec2-user@" + ip_address_str;
        std::string download_install_script = ssh_base + " wget https://ec2setup.s3.us-east-2.amazonaws.com/ec2_setup.sh /home/ec2-user/ec2_setup.sh";
        std::string run_install_script = ssh_base + " sh /home/ec2-user/ec2_setup.sh";
        std::string query_running_jupyter_sessions = ssh_base + " jupyter notebook list";
        exec(download_install_script.c_str());
        exec(run_install_script.c_str());

        // Get notebook URL
        std::string running_notebook_sessions = exec(query_running_jupyter_sessions.c_str());
        auto url_start_idx = running_notebook_sessions.find("http://");
        auto url_end_idx = running_notebook_sessions.find(" :: ");
        notebook_token_url = running_notebook_sessions.substr(url_start_idx, url_end_idx - url_start_idx);
        std::cout << notebook_token_url << std::endl;

        // Open detached thread to keep notebook connection open
        std::string open_jupyter_connection = "ssh -CNL localhost:5678:localhost:5678 -i " + this->notebookConfig.keyPath + " ec2-user@" + ip_address_str;
        std::thread(this->open_ssh_notebook_tunnel, open_jupyter_connection).detach();

        this->notebookConfig.price = price;
        this->notebookConfig.instanceId = instance_id;
        this->notebookConfig.notebookUrl = notebook_token_url;
        this->notebookConfig.publicIp = ip_address;

    }
    Aws::ShutdownAPI(options);

}

bool AwsUtils::TerminateInstance() {

    bool termination_is_successful;
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        // Set up client
        Aws::Client::ClientConfiguration client_config;
        client_config.region = Aws::Region::US_EAST_2;
        Aws::EC2::EC2Client ec2_client(client_config);

        // Terminate instance
        Aws::EC2::Model::TerminateInstancesRequest termination_request;
        termination_request.SetInstanceIds(this->CastToAwsStringVector(this->notebookConfig.instanceId));
        auto termination_response = ec2_client.TerminateInstances(termination_request);
        termination_is_successful = termination_response.GetResult().GetTerminatingInstances().size() > 0;
    }
    Aws::ShutdownAPI(options);

    return termination_is_successful;
}

void AwsUtils::RefreshConnection() {
    // Open detached thread to keep notebook connection open
    std::string open_jupyter_connection = "ssh -CNL localhost:5678:localhost:5678 -i " + this->notebookConfig.keyPath + " ec2-user@" + this->notebookConfig.publicIp;
    std::thread(this->open_ssh_notebook_tunnel, open_jupyter_connection).detach();
}

void AwsUtils::CreateKeyPair() {

    // Precautionary clean up
    this->DeleteKeyPair();

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        // Set up client
        Aws::Client::ClientConfiguration client_config;
        client_config.region = Aws::Region::US_EAST_2;
        Aws::EC2::EC2Client ec2_client(client_config);

        // Create Key Pair
        Aws::EC2::Model::CreateKeyPairRequest create_key_request;
        create_key_request.SetKeyName(this->notebookConfig.keyName.c_str());
        auto create_key_result = ec2_client.CreateKeyPair(create_key_request);
        auto key_material = create_key_result.GetResult().GetKeyMaterial();

        // Write Key Pair to File
        auto home_dir = std::getenv("HOME");
        std::filesystem::path diogenes_dir(std::filesystem::path(home_dir) / std::filesystem::path(".diogenes"));
        std::filesystem::path key_file_path(diogenes_dir / std::filesystem::path(this->notebookConfig.keyName + ".pem"));
        std::filesystem::create_directory(diogenes_dir);
        std::ofstream key_file;
        key_file.open(key_file_path.c_str());
        key_file << key_material;
        key_file.close();
        std::filesystem::permissions(key_file_path, std::filesystem::perms::owner_all);
        this->notebookConfig.keyPath = key_file_path.c_str();
    }
    Aws::ShutdownAPI(options);
}

void AwsUtils::DeleteKeyPair() {

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        // Set up client
        Aws::Client::ClientConfiguration client_config;
        client_config.region = Aws::Region::US_EAST_2;
        Aws::EC2::EC2Client ec2_client(client_config);

        // Delete Key Pair
        Aws::EC2::Model::DeleteKeyPairRequest delete_key_request;
        delete_key_request.SetKeyName(this->notebookConfig.keyName.c_str());
        auto delete_key_result = ec2_client.DeleteKeyPair(delete_key_request);

        // Remove Key Pair File
        auto home_dir = std::getenv("HOME");
        std::filesystem::path diogenes_dir(std::filesystem::path(home_dir) / std::filesystem::path(".diogenes"));
        std::filesystem::path key_file_path(diogenes_dir / std::filesystem::path(this->notebookConfig.keyName + ".pem"));
        std::filesystem::remove(key_file_path);
    }
    Aws::ShutdownAPI(options);

}


