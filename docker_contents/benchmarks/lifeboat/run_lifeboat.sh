#!/bin/bash

config=$1

if [[ ! -e ${config} ]]; then
    echo "Config file \"${config}\" doesn't exist."
    exit 1
fi

echo "Using config file ${config}..."
echo "Contents:"
cat $config
. $config

export DARSHAN_ENABLE_NONMPI=1
env LD_PRELOAD=/usr/local/lib/libdarshan.so ./out/lifeboat-flat ${MODE} ${DIM0} ${DIM1} 
