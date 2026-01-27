#!/bin/bash

config=$1

if [[ -z ${OUTPUT_DIR} ]]; then
    echo "Variable \"OUTPUT_DIR\" not set. Please set to specifiy where to output benchmark to."
    exit 1
fi

mkdir ${OUTPUT_DIR}/adios

if [[ ! -e ${config} ]]; then
    echo "Config file \"${config}\" doesn't exist."
    exit 1
fi

echo "Using config file ${config}..."
echo "Contents:"
cat $config
. $config

export MPICH_NEMESIS_NETMOD=ch3:sock
mpiexec -n 16 --host localhost ./out/adios-benchmark ${MODE} ${DIM0} ${DIM1} > ${OUTPUT_DIR}/adios/$(basename ${config})-out.txt

