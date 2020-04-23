# Diogenes

## Overview

The cloud product broke sages have been waiting for! This app runs remote Jupyter Notebooks cheap and easy using AWS Spot instances. It essentially offers a frugal alternative to running notebooks through the AWS SageMaker service. By using Spot instances instead of EC2 On-Demand pricing, the user can shave 60-80% off of their SageMaker bill.

<img src="https://github.com/maxlamberti/Diogenes/blob/master/resources/diogenes.png" width="500">
The original broke sage Diogenes when the SageMaker bill collector comes (*Est. 320 BC*).

## Status

In development. Expected first release in May 2020.

## Scope & Disclaimer

Diogenes takes care of setting up the instance, installing required packages, and opening the jupyter connection. No third-party sign-ups or purchases are required, everything happens through the user's AWS account. Any notebook launched through Diogenes will incur costs to the user's AWS bill.

## Tech

- C++ AWS-SDK
- EC2 Spot Hosting
- Qt (GUI)
- Jupyter
- MacOS App Bundle
