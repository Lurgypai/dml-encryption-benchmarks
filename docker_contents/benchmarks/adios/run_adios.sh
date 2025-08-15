#!/bin/bash

config=$1

mkdir -p /workspace/output/adios

if [[ ! -e ${config} ]]; then
    echo "Config file \"${config}\" doesn't exist."
    exit 1
fi

echo "Using config file ${config}..."
echo "Contents:"
cat $config
. $config

export DARSHAN_ENABLE_NONMPI=1
env LD_PRELOAD=/usr/local/lib/libdarshan.so ./out/adios-benchmark ${MODE} ${DIM0} ${DIM1} > /workspace/output/adios/$(basename ${config})-out.txt
