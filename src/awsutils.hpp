#ifndef BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP
#define BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP


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



class AwsUtils {

 private:
  Aws::SDKOptions SDKOptions;
  Aws::EC2::EC2Client EC2Client;
  template <typename T> std::vector<Aws::String> MapEnumVecToSortedStrVec(std::vector<T> input_vector, Aws::String (*mapper)(T));

 public:
  AwsUtils();
  ~AwsUtils();
  void SetClientConfiguration(Aws::Client::ClientConfiguration client_config);
  std::vector<Aws::String> GetSpotInstanceTypes();

};

#endif //BROKE_SAGE_NOTEBOOKS_AWSUTILS_HPP
