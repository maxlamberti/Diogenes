#ifndef BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP
#define BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP


#include <map>
#include <string>
#include <iostream>
#include <algorithm>
#include <aws/core/Aws.h>
#include <aws/ec2/EC2Client.h>
#include <aws/ec2/EC2Request.h>
#include <aws/ec2/model/DescribeImagesRequest.h>
#include <aws/ec2/model/DescribeInstanceTypesRequest.h>
#include <aws/ec2/model/DescribeSpotInstanceRequestsRequest.h>


class AwsUtils {

 private:
  Aws::SDKOptions SDKOptions;
  Aws::EC2::EC2Client EC2Client;
  template <typename T> std::vector<Aws::String> MapEnumVecToSortedStrVec(std::vector<T> input_vector, Aws::String (*mapper)(T));
  Aws::Vector<Aws::String> CastToAwsStringVector(const std::string& str);
  Aws::EC2::Model::SummaryStatus GetInstanceStatus(const std::string& instance_id, const Aws::EC2::EC2Client& ec2_client);
  std::string GetInstanceId(const Aws::Vector<Aws::String>& request_id, const Aws::EC2::EC2Client& ec2_client);

 public:
  struct NotebookConfig {
      std::string imageId, keyName;
      Aws::EC2::Model::InstanceType instanceType;
  };
  AwsUtils();
  ~AwsUtils();
  std::string instanceType;
  NotebookConfig notebookConfig;
  void setClientConfiguration(Aws::Client::ClientConfiguration client_config);
  std::vector<Aws::String> getSpotInstanceTypes();
  void launchSpotInstance();

};

#endif //BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP
