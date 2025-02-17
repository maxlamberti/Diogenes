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
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/ec2/model/CreateKeyPairRequest.h>
#include <aws/ec2/model/DeleteKeyPairRequest.h>
#include <aws/ec2/model/DescribeImagesRequest.h>
#include <aws/ec2/model/DescribeInstancesRequest.h>
#include <aws/ec2/model/TerminateInstancesRequest.h>
#include <aws/ec2/model/CreateSecurityGroupRequest.h>
#include <aws/ec2/model/DeleteSecurityGroupRequest.h>
#include <aws/ec2/model/RequestSpotInstancesRequest.h>
#include <aws/ec2/model/DescribeInstanceTypesRequest.h>
#include <aws/ec2/model/DescribeInstanceStatusRequest.h>
#include <aws/ec2/model/DescribeSpotInstanceRequestsRequest.h>
#include <aws/ec2/model/AuthorizeSecurityGroupIngressRequest.h>
#include <aws/ec2/model/CancelSpotInstanceRequestsRequest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "awsutils.hpp"


using namespace Aws::EC2::Model;

const auto DEFAULT_INSTANCE_TYPE = InstanceType::t2_micro;
const std::string DEFAULT_KEY_NAME("DiogenesKey");
const std::string DEFAULT_SEC_GROUP_NAME("DiogenesSecGroup");
const int MIN_BLOCK_SIZE = 8;  // GB
const bool DEFAULT_DELETE_STORAGE_STATE = true;
const std::string DEFAULT_REGION = "us-east-1";
const std::set<std::string> AVAILABLE_REGIONS = {
    "us-east-1", "us-east-2", "us-west-1", "us-west-2", "ca-central-1",
    "eu-west-1", "eu-west-2", "eu-west-3", "eu-central-1",
    "ap-southeast-1", "ap-southeast-2", "ap-northeast-1", "ap-northeast-2",
    "ap-south-1", "sa-east-1"
};
const std::set<std::string> HEALTHY_REQUEST_STATES = {
    "pending-evaluation", "pending-fulfillment", "fulfilled"
};

std::string ShellExecute(const char* cmd) {
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

    this->notebookConfig.instanceType = DEFAULT_INSTANCE_TYPE;
    this->notebookConfig.keyName = DEFAULT_KEY_NAME;
    this->notebookConfig.secGroupName = DEFAULT_SEC_GROUP_NAME;
    this->AvailableRegions = AVAILABLE_REGIONS;
    this->notebookConfig.deleteStorage = DEFAULT_DELETE_STORAGE_STATE;
    this->notebookConfig.blockSize = MIN_BLOCK_SIZE;  // GB
    this->notebookConfig.region = DEFAULT_REGION;
    this->notebookConfig.hasSystemCredentials = this->HasCredentials();
    this->client_config = Aws::Client::ClientConfiguration();
    this->client_config.region = DEFAULT_REGION;

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
template std::vector<Aws::String> AwsUtils::MapEnumVecToSortedStrVec<InstanceType>(std::vector<InstanceType>,
                                                                                                    Aws::String (*)(InstanceType));

auto AwsUtils::GetSpotInstanceTypes() -> std::vector<Aws::String> {

    std::vector<Aws::String> all_instance_types;

    // Set up client
    auto ec2_client = this->GetClient();

    // Get response from AWS
    DescribeInstanceTypesRequest instance_type_request;
    auto instance_type_response = ec2_client.DescribeInstanceTypes(instance_type_request);
    auto instance_types_in_response = instance_type_response.GetResult().GetInstanceTypes();

    // Map response to string vector
    auto instance_type_mapper = InstanceTypeMapper::GetNameForInstanceType;
    std::vector<InstanceType> instance_type_vec;
    instance_type_vec.reserve(instance_types_in_response.size());
    for (auto val : instance_types_in_response) {
        instance_type_vec.push_back(val.GetInstanceType());
    }
    all_instance_types = this->MapEnumVecToSortedStrVec(instance_type_vec, instance_type_mapper);

    return all_instance_types;

};

bool AwsUtils::HasCredentials() {

    Aws::Auth::ProfileConfigFileAWSCredentialsProvider provider;
    bool has_credentials = not provider.GetAWSCredentials().IsEmpty();

    return has_credentials;
}

void AwsUtils::SetCredentials(std::string access_key_id, std::string secret_key) {
    this->notebookConfig.credentials = Aws::Auth::AWSCredentials(access_key_id, secret_key);
}

Aws::EC2::EC2Client AwsUtils::GetClient() {

    Aws::EC2::EC2Client ec2_client;
    if (this->notebookConfig.hasSystemCredentials) {
        ec2_client = Aws::EC2::EC2Client(this->client_config);
    } else {
        ec2_client = Aws::EC2::EC2Client(this->notebookConfig.credentials, this->client_config);
    }

    return ec2_client;
}


Aws::Vector<Aws::String> AwsUtils::CastToAwsStringVector(const std::string& str) {
    Aws::Vector<Aws::String> result_vec = {Aws::String(str)};
    return result_vec;
}

std::string AwsUtils::GetInstanceId(const Aws::Vector<Aws::String>& request_id, const Aws::EC2::EC2Client& ec2_client) {

    // Query for instance id
    DescribeSpotInstanceRequestsRequest describe_spot_instance_request;
    describe_spot_instance_request.SetSpotInstanceRequestIds(request_id);
    auto describe_instance_response = ec2_client.DescribeSpotInstanceRequests(describe_spot_instance_request);

    std::string instance_id;
    auto instance_description_vec = describe_instance_response.GetResult().GetSpotInstanceRequests();
    if (instance_description_vec.size() != 0) {
        instance_id = instance_description_vec[0].GetInstanceId().c_str();
    }

    return instance_id;
};


SummaryStatus AwsUtils::GetInstanceStatus(const std::string& instance_id, const Aws::EC2::EC2Client& ec2_client) {

    // Query for instance status
    DescribeInstanceStatusRequest instance_status_request;
    auto aws_instance_id = this->CastToAwsStringVector(instance_id.c_str());
    instance_status_request.SetInstanceIds(aws_instance_id);
    auto instance_status_response = ec2_client.DescribeInstanceStatus(instance_status_request);

    // Sometimes response is empty
    auto instance_status_vec = instance_status_response.GetResult().GetInstanceStatuses();
    SummaryStatus instance_status;
    if (instance_status_vec.size() != 0) {
        for (auto val : instance_status_vec) {
            instance_status = val.GetInstanceStatus().GetStatus();
        }
        instance_status = instance_status_vec[0].GetInstanceStatus().GetStatus();
    } else {
        instance_status = SummaryStatus::NOT_SET;
    }

    return instance_status;
}

bool AwsUtils::IsGpuInstance(InstanceType instance_type) {

    bool is_gpu_instance;
    std::string instance_type_str(InstanceTypeMapper::GetNameForInstanceType(instance_type));
    char first_letter(instance_type_str.at(0));
    std::string first_three_letters = instance_type_str.substr(0, 3);

    // p__, g__ or inf machines have GPUs
    is_gpu_instance = (first_letter == 'p') | (first_letter =='g') | (first_three_letters.compare("inf") == 0);

    return is_gpu_instance;
}

void AwsUtils::OpenSshNotebookTunnel(std::string cmd) {
    ShellExecute(cmd.c_str());
}

RequestSpotLaunchSpecification AwsUtils::GetLaunchSpecification() {

    // Get ec2 client
    auto ec2_client = this->GetClient();

    // Configure root block storage device
    DescribeImagesRequest describe_image_request;
    describe_image_request.SetImageIds(this->CastToAwsStringVector(this->notebookConfig.imageId));
    auto describe_image_response = ec2_client.DescribeImages(describe_image_request);
    auto block_name = describe_image_response.GetResult().GetImages()[0].GetBlockDeviceMappings()[0].GetDeviceName();
    EbsBlockDevice root_device;
    BlockDeviceMapping root_device_mapping;
    root_device_mapping.SetDeviceName(block_name);
    root_device.SetDeleteOnTermination(this->notebookConfig.deleteStorage);
    root_device.SetVolumeSize(std::max(MIN_BLOCK_SIZE, this->notebookConfig.blockSize));
    root_device_mapping.SetEbs(root_device);

    // Set launch specifications
    RequestSpotLaunchSpecification launch_spec;
    launch_spec.SetInstanceType(this->notebookConfig.instanceType);
    launch_spec.SetKeyName(this->notebookConfig.keyName.c_str());
    launch_spec.SetImageId(this->notebookConfig.imageId.c_str());
    launch_spec.SetSecurityGroups(this->CastToAwsStringVector(this->notebookConfig.secGroupName));
    launch_spec.AddBlockDeviceMappings(root_device_mapping);

    return launch_spec;
}


Aws::EC2::Model::RequestSpotInstancesResponse AwsUtils::RequestInstance(RequestSpotLaunchSpecification launch_spec) {

    // Request spot instance
    auto ec2_client = this->GetClient();
    RequestSpotInstancesRequest spot_details;
    spot_details.SetInstanceCount(1);
    spot_details.SetLaunchSpecification(launch_spec);
    auto response = ec2_client.RequestSpotInstances(spot_details).GetResult();

    return response;
}

std::string AwsUtils::MonitorInstanceLaunch() {

    // Set up client and data
    auto ec2_client = this->GetClient();
    auto request_id = this->CastToAwsStringVector(this->notebookConfig.requestId.c_str());

    // Query instance status until it's ready to connect to
    std::string instance_id;
    SpotInstanceStatus request_status;  // this status is relevant before EC2 gets launched
    SummaryStatus instance_status;  // this status is relevant after launch
    while (true) {

        // If request cannot be fulfilled cancel
        request_status = this->GetSpotRequestStatus(this->notebookConfig.requestId, ec2_client);
        if (HEALTHY_REQUEST_STATES.find(request_status.GetCode().c_str()) == HEALTHY_REQUEST_STATES.end()) {
            CancelSpotInstanceRequestsRequest cancel_spot_request_request;
            cancel_spot_request_request.SetSpotInstanceRequestIds(request_id);
            ec2_client.CancelSpotInstanceRequests(cancel_spot_request_request);
            throw std::runtime_error(request_status.GetMessage().c_str());
        }

        if (instance_id.size() == 0) {
            instance_id = this->GetInstanceId(request_id, ec2_client);
        }

        instance_status = this->GetInstanceStatus(instance_id, ec2_client);
        if (instance_status == SummaryStatus::ok) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds (10));
    }

    return instance_id;
}

std::string AwsUtils::GetInstanceIpAddress() {

    // Get public IP address of instance
    auto ec2_client = this->GetClient();
    Filter filter;
    filter.SetName("instance-id");
    filter.SetValues(this->CastToAwsStringVector(this->notebookConfig.instanceId));
    DescribeInstancesRequest describe_instance_request;
    describe_instance_request.AddFilters(filter);
    auto describe_instances_response = ec2_client.DescribeInstances(describe_instance_request);
    auto ip_address = describe_instances_response.GetResult().GetReservations()[0].GetInstances()[0].GetPublicIpAddress();
    std::string ip_address_str = ip_address.c_str();

    return ip_address_str;
}

void AwsUtils::RunInstanceInstallScripts() {

    // Run install scripts
    std::string install_script = this->notebookConfig.isGpuInstance? "ec2_dl_ami_setup.sh" : "ec2_amzn2_ami_setup.sh";
    std::string ssh_base = "ssh -oStrictHostKeyChecking=no -i " + this->notebookConfig.keyPath + " ec2-user@" + this->notebookConfig.publicIp;
    std::string download_install_script = ssh_base + " wget https://ec2setup.s3.us-east-2.amazonaws.com/" + install_script + " /home/ec2-user/" + install_script;
    std::string run_install_script = ssh_base + " sh /home/ec2-user/" + install_script;
    ShellExecute(download_install_script.c_str());
    ShellExecute(run_install_script.c_str());  // TODO: run asynchronously

}

std::string AwsUtils::GetNotebookUrl() {

    // Get notebook URL
    std::string ssh_base = "ssh -oStrictHostKeyChecking=no -i " + this->notebookConfig.keyPath + " ec2-user@" + this->notebookConfig.publicIp;
    std::string query_running_jupyter_sessions = ssh_base + " jupyter notebook list";
    std::string running_notebook_sessions;
    while (true) {
        running_notebook_sessions = ShellExecute(query_running_jupyter_sessions.c_str());
        if (running_notebook_sessions.find("http:") != std::string::npos) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds (10));
    }
    auto url_start_idx = running_notebook_sessions.find("http://");
    auto url_end_idx = running_notebook_sessions.find(" :: ");
    auto notebook_token_url = running_notebook_sessions.substr(url_start_idx, url_end_idx - url_start_idx);

    return notebook_token_url;
}

void AwsUtils::LaunchSpotInstance() {

    // Determine instance type and corresponding imageId
    this->notebookConfig.isGpuInstance = this->IsGpuInstance(this->notebookConfig.instanceType);
    this->notebookConfig.imageId = this->GetImageId(this->notebookConfig.isGpuInstance);

    // Request spot instance
    auto launch_spec = GetLaunchSpecification();
    auto response = this->RequestInstance(launch_spec);

    // Get initial data and request ID
    auto response_data = response.GetSpotInstanceRequests();
    this->notebookConfig.requestId = response_data[0].GetSpotInstanceRequestId();
    auto request_id = this->CastToAwsStringVector(this->notebookConfig.requestId.c_str());
    this->notebookConfig.price = response_data[0].GetSpotPrice();

    // Query instance status until it's ready to connect to
    this->notebookConfig.instanceId = this->MonitorInstanceLaunch();
    this->notebookConfig.publicIp = this->GetInstanceIpAddress();

    // Run instance install scripts and get notebook URL
    this->RunInstanceInstallScripts();
    this->notebookConfig.notebookUrl = this->GetNotebookUrl();

    // Open detached thread to keep notebook connection open
    std::string open_jupyter_connection = "ssh -CNL localhost:5678:localhost:5678 -i " + this->notebookConfig.keyPath + " ec2-user@" + this->notebookConfig.publicIp;
    std::thread(this->OpenSshNotebookTunnel, open_jupyter_connection).detach();
}

bool AwsUtils::TerminateInstance() {

    bool termination_is_successful;

    // Set up client
    auto ec2_client = this->GetClient();

    // Terminate instance
    TerminateInstancesRequest termination_request;
    termination_request.SetInstanceIds(this->CastToAwsStringVector(this->notebookConfig.instanceId));
    auto termination_response = ec2_client.TerminateInstances(termination_request);
    termination_is_successful = termination_response.GetResult().GetTerminatingInstances().size() > 0;

    return termination_is_successful;
}

void AwsUtils::RefreshConnection() {
    // Open detached thread to keep notebook connection open
    std::string open_jupyter_connection = "ssh -CNL localhost:5678:localhost:5678 -i " + this->notebookConfig.keyPath + " ec2-user@" + this->notebookConfig.publicIp;
    std::thread(this->OpenSshNotebookTunnel, open_jupyter_connection).detach();
}

void AwsUtils::CreateKeyPair() {

    // Precautionary clean up
    this->DeleteKeyPair();

    // Set up client
    auto ec2_client = this->GetClient();

    // Create Key Pair
    CreateKeyPairRequest create_key_request;
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

void AwsUtils::DeleteKeyPair() {

    // Set up client
    auto ec2_client = this->GetClient();

    // Delete Key Pair
    DeleteKeyPairRequest delete_key_request;
    delete_key_request.SetKeyName(this->notebookConfig.keyName.c_str());
    auto delete_key_result = ec2_client.DeleteKeyPair(delete_key_request);

    // Remove Key Pair File
    auto home_dir = std::getenv("HOME");
    std::filesystem::path diogenes_dir(std::filesystem::path(home_dir) / std::filesystem::path(".diogenes"));
    std::filesystem::path key_file_path(diogenes_dir / std::filesystem::path(this->notebookConfig.keyName + ".pem"));
    std::filesystem::remove(key_file_path);

}

void AwsUtils::CreateSecurityGroup() {

    // Set up client
    auto ec2_client = this->GetClient();

    // Create security group
    CreateSecurityGroupRequest create_security_group_request;
    create_security_group_request.SetGroupName(this->notebookConfig.secGroupName.c_str());
    create_security_group_request.SetDescription("Diogenes security group. Open to public ssh on port 22.");
    auto create_security_group_response = ec2_client.CreateSecurityGroup(create_security_group_request);

    // Open port 22 on security group
    AuthorizeSecurityGroupIngressRequest auth_ingress_request;
    auth_ingress_request.SetGroupName(this->notebookConfig.secGroupName.c_str());
    IpRange ip_range;
    IpPermission ip_permission;
    ip_range.SetCidrIp("0.0.0.0/0");
    ip_permission.SetIpProtocol("tcp");
    ip_permission.SetToPort(22);
    ip_permission.SetFromPort(22);
    ip_permission.AddIpRanges(ip_range);
    auth_ingress_request.AddIpPermissions(ip_permission);
    auto auth_ingress_response = ec2_client.AuthorizeSecurityGroupIngress(auth_ingress_request);

}

void AwsUtils::DeleteSecurityGroup() {

    // Set up client
    auto ec2_client = this->GetClient();

    // Delete Sec Group
    DeleteSecurityGroupRequest delete_security_group_request;
    delete_security_group_request.SetGroupName(this->notebookConfig.secGroupName.c_str());
    auto delete_security_group_response = ec2_client.DeleteSecurityGroup(delete_security_group_request);

}

SpotInstanceStatus AwsUtils::GetSpotRequestStatus(const std::string& request_id, const Aws::EC2::EC2Client& ec2_client) {

    // Get status
    DescribeSpotInstanceRequestsRequest describe_spot_request_request;
    describe_spot_request_request.SetSpotInstanceRequestIds(this->CastToAwsStringVector(request_id));
    auto describe_spot_request_result = ec2_client.DescribeSpotInstanceRequests(describe_spot_request_request);
    SpotInstanceStatus status = describe_spot_request_result.GetResult().GetSpotInstanceRequests()[0].GetStatus();

    return status;
}


std::string AwsUtils::GetImageId(bool is_gpu_instance) {

    // Select image depending on gpu
    std::string image_name, image_id;
    if (is_gpu_instance) {
        image_name = "Deep Learning AMI (Amazon Linux 2)*";
    } else {
        image_name = "amzn2-ami-hvm-2.0.????????.?-x86_64-gp2";
    }

    // Set up client
    auto ec2_client = this->GetClient();

    // Query for suitable machine images
    DescribeImagesRequest describe_images_request;
    Filter name_filter, state_filter;
    state_filter.SetName("state");
    state_filter.SetValues(this->CastToAwsStringVector("available"));
    name_filter.SetName("name");
    name_filter.SetValues(this->CastToAwsStringVector(image_name));
    describe_images_request.AddFilters(name_filter);
    describe_images_request.AddFilters(state_filter);
    describe_images_request.SetOwners(CastToAwsStringVector("amazon"));
    auto describe_images_result = ec2_client.DescribeImages(describe_images_request);
    auto viable_images = describe_images_result.GetResult().GetImages();

    // Find the newest image by taking max of the name (date formatting permits max operation)
    std::vector<std::string> instance_name_vec;
    instance_name_vec.reserve(viable_images.size());
    for (auto image : viable_images) {
        instance_name_vec.push_back(image.GetName().c_str());
    }
    auto max_iterator = std::max_element(instance_name_vec.begin(), instance_name_vec.end());
    int max_index = std::distance(instance_name_vec.begin(), max_iterator);
    image_id = viable_images[max_index].GetImageId();

    return image_id;
}

void AwsUtils::ResetConfigParameters() {
    this->notebookConfig.publicIp = "";
    this->notebookConfig.notebookUrl = "";
    this->notebookConfig.keyPath = "";
    this->notebookConfig.instanceId = "";
    this->notebookConfig.instanceType = DEFAULT_INSTANCE_TYPE;
    this->notebookConfig.price = "";
}
