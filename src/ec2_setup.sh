#!/bin/bash

# Updates and other installations 
sudo yum update -y
sudo yum install -y git screen htop

# Install Anaconda
wget https://repo.continuum.io/archive/Anaconda3-2020.02-Linux-x86_64.sh -O ~/anaconda.sh
bash ~/anaconda.sh -b -p $HOME/anaconda
rm ~/anaconda.sh
eval "$($HOME/anaconda/bin/conda shell.bash hook)"
conda init bash

# Start persistent jupyter lab session
screen -d -m jupyter lab --no-browser --port=5678

# View connection notebook link/token by running the next line on ec2 machine:
# jupyter notebook list

# Run next line on local machine to connect
# ssh -CNL 5678:localhost:5678 -i PATH/TO/SSH/KEY ec2-user@EC2s.PUBLIC.IP





