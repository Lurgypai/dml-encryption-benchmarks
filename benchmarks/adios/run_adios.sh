#!/bin/bash

config=$1

USE_STDOUT=""

if [[ -z ${OUTPUT_DIR} ]]; then
    echo "Variable \"OUTPUT_DIR\" not set, printing to stdout"
    USE_STDOUT="yes"
fi

if [[ ! $USE_STDOUT ]]; then
    mkdir ${OUTPUT_DIR}/adios
fi

if [[ ! -e ${config} ]]; then
    echo "Config file \"${config}\" doesn't exist."
    exit 1
fi

echo "Using config file ${config}..."
echo "Contents:"
cat $config
. $config

# export UCX_TLS=self
# MPIRUN="mpiexec -n 1"
#
MPIRUN=srun

if [[ ! $USE_STDOUT ]]; then
    ${MPIRUN} ./out/adios-benchmark ${MODE} ${RW} ${DIM0} ${DIM1} > ${OUTPUT_DIR}/adios/$(basename ${config}).txt
else
    ${MPIRUN} ./out/adios-benchmark ${MODE} ${RW} ${DIM0} ${DIM1}
fi
