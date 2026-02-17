#!/bin/bash

config=$1

USE_STDOUT=""

if [[ -z ${OUTPUT_DIR} ]]; then
    echo "Variable \"OUTPUT_DIR\" not set, printing to stdout"
    USE_STDOUT="yes"
fi

rm -r benchmark_out.bp
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

export UCX_TLS=self
# MPIRUN=srun
MPIRUN="mpiexec -n 1"

if [[ ! $USE_STDOUT ]]; then
    ${MPIRUN} ./out/adios-benchmark ${MODE} write ${DIM0} ${DIM1} > ${OUTPUT_DIR}/adios/$(basename ${config})-write.txt
    ${MPIRUN} ./out/adios-benchmark ${MODE} read ${DIM0} ${DIM1} > ${OUTPUT_DIR}/adios/$(basename ${config})-read.txt
else
    ${MPIRUN} ./out/adios-benchmark ${MODE} write ${DIM0} ${DIM1}
    ${MPIRUN} ./out/adios-benchmark ${MODE} read ${DIM0} ${DIM1}
fi
