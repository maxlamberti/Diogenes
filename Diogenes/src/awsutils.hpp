#ifndef BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP
#define BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP


#include <string>
#include <aws/core/Aws.h>
#include <aws/ec2/EC2Client.h>
#include <aws/ec2/EC2Request.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/ec2/model/DescribeImagesRequest.h>
#include <aws/ec2/model/DescribeInstanceTypesRequest.h>
#include <aws/ec2/model/DescribeSpotInstanceRequestsRequest.h>
#include <aws/ec2/model/RequestSpotLaunchSpecification.h>


struct NotebookConfig {
    std::string imageId, keyName, keyPath, instanceId, price, notebookUrl,
    publicIp, secGroupName, region, requestId;
    bool isGpuInstance, deleteStorage, hasSystemCredentials;
    int blockSize;
    Aws::EC2::Model::InstanceType instanceType;
    Aws::Auth::AWSCredentials credentials;
};

class AwsUtils {

 private:
  Aws::Client::ClientConfiguration client_config;
  template <typename T> std::vector<Aws::String> MapEnumVecToSortedStrVec(std::vector<T> input_vector, Aws::String (*mapper)(T));
  Aws::Vector<Aws::String> CastToAwsStringVector(const std::string& str);
  Aws::EC2::Model::SummaryStatus GetInstanceStatus(const std::string& instance_id, const Aws::EC2::EC2Client& ec2_client);
  Aws::EC2::Model::SpotInstanceStatus GetSpotRequestStatus(const std::string& request_id, const Aws::EC2::EC2Client& ec2_client);
  std::string GetInstanceId(const Aws::Vector<Aws::String>& request_id, const Aws::EC2::EC2Client& ec2_client);
  static bool IsGpuInstance(Aws::EC2::Model::InstanceType instance_type);
  static void OpenSshNotebookTunnel(std::string cmd);
  std::string GetImageId(bool is_gpu_instance);
  void InitializeRegion();
  Aws::EC2::EC2Client GetClient();
  Aws::EC2::Model::RequestSpotLaunchSpecification GetLaunchSpecification();
  std::string MonitorInstanceLaunch();
  std::string GetInstanceIpAddress();
  Aws::EC2::Model::RequestSpotInstancesResponse RequestInstance(Aws::EC2::Model::RequestSpotLaunchSpecification launch_spec);
  void RunInstanceInstallScripts();
  std::string GetNotebookUrl();
  bool HasCredentials();

public:
  AwsUtils();
  ~AwsUtils();
  NotebookConfig notebookConfig;
  std::set<std::string> AvailableRegions;
  void LaunchSpotInstance();
  bool TerminateInstance();
  void RefreshConnection();
  void CreateKeyPair();
  void DeleteKeyPair();
  void CreateSecurityGroup();
  void DeleteSecurityGroup();
  std::vector<Aws::String> GetSpotInstanceTypes();
  void ResetConfigParameters();
  void SetCredentials(std::string access_key_id, std::string secret_key);

};

#endif //BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP
