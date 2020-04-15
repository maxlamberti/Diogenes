#ifndef BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP
#define BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP


#include <string>
#include <aws/core/Aws.h>
#include <aws/ec2/EC2Client.h>
#include <aws/ec2/EC2Request.h>
#include <aws/ec2/model/DescribeImagesRequest.h>
#include <aws/ec2/model/DescribeInstanceTypesRequest.h>
#include <aws/ec2/model/DescribeSpotInstanceRequestsRequest.h>


struct NotebookConfig {
    std::string imageId, keyName, keyPath, instanceId, price, notebookUrl,
    publicIp, secGroupName, region;
    bool isGpuInstance, deleteStorage;
    int blockSize;
    Aws::EC2::Model::InstanceType instanceType;
};

class AwsUtils {

 private:
  Aws::SDKOptions SDKOptions;
  template <typename T> std::vector<Aws::String> MapEnumVecToSortedStrVec(std::vector<T> input_vector, Aws::String (*mapper)(T));
  Aws::Vector<Aws::String> CastToAwsStringVector(const std::string& str);
  Aws::EC2::Model::SummaryStatus GetInstanceStatus(const std::string& instance_id, const Aws::EC2::EC2Client& ec2_client);
  Aws::EC2::Model::SpotInstanceStatus GetSpotRequestStatus(const std::string& request_id, const Aws::EC2::EC2Client& ec2_client);
  std::string GetInstanceId(const Aws::Vector<Aws::String>& request_id, const Aws::EC2::EC2Client& ec2_client);
  static bool IsGpuInstance(Aws::EC2::Model::InstanceType instance_type);
  static void OpenSshNotebookTunnel(std::string cmd);
  std::string GetImageId(bool is_gpu_instance);
  void InitializeRegion();

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

};

#endif //BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP
