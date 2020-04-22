#!/bin/bash

# Updates and other installations
# sudo yum update -y
# sudo yum install -y git screen htop

# Set up anaconda
sudo ln -s /home/ec2-user/anaconda3/etc/profile.d/conda.sh /etc/profile.d/conda.sh
echo 'conda activate base' >> $HOME/.bashrc

# Start persistent jupyter lab session
pip uninstall -y jupyterlab  # reinstall because: https://github.com/jupyterlab/jupyterlab/issues/7959
pip install jupyterlab
screen -d -m jupyter lab --no-browser --port=5678

# Clean up
rm $HOME/ec2_dl_ami_setup.sh
rm -rf $HOME/tutorials/
rm -rf $HOME/examples/

# View connection notebook link/token by running the next line on ec2 machine:
# jupyter notebook list

# Run next line on local machine to connect
# ssh -CNL 5678:localhost:5678 -i PATH/TO/SSH/KEY ec2-user@EC2s.PUBLIC.IP
