#!/usr/bin/env bash

trap 'exit' ERR

cd /svcLossSim

if [ -d "bin/" ]; then
	mv bin/ bin_$(date -d "today" +"%Y%m%d%H%M%S")
fi

mkdir bin
cd bin
cmake ../
make

echo "Provision (user) completed."
