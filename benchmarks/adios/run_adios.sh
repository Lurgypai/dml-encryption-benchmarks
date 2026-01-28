#!/bin/bash

config=$1

if [[ -z ${OUTPUT_DIR} ]]; then
    echo "Variable \"OUTPUT_DIR\" not set. Please set to specifiy where to output benchmark to."
    exit 1
fi

rm -r benchmark_out.bp
mkdir ${OUTPUT_DIR}/adios

if [[ ! -e ${config} ]]; then
    echo "Config file \"${config}\" doesn't exist."
    exit 1
fi

echo "Using config file ${config}..."
echo "Contents:"
cat $config
. $config

export UCX_TLS=self
# MPIRUN=srun
MPIRUN="../../dependencies/mpich-ins/bin/mpiexec -n 16"
${MPIRUN} ./out/adios-benchmark ${MODE} ${DIM0} ${DIM1} > ${OUTPUT_DIR}/adios/$(basename ${config})-out.txt

