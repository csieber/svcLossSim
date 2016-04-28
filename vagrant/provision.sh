#!/usr/bin/env bash

trap 'exit' ERR

apt-get update

apt-get -y install g++ cmake libboost-system-dev libboost-program-options-dev libboost-thread-dev

ln -s /svcLossSim/bin/bin/svclosssim /usr/bin/

echo "Provision (root) completed."
