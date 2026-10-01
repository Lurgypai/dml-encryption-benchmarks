#!/bin/bash

config=$1

USE_STDOUT=""

if [[ -z ${OUTPUT_DIR} ]]; then
    echo "Variable \"OUTPUT_DIR\" not set, printing to stdout"
    USE_STDOUT="yes"
fi

if [[ ! $USE_STDOUT ]]; then
    if [[ ! -e ${OUTPUT_DIR}/adios ]]; then
        mkdir ${OUTPUT_DIR}/enc_wrapper
    fi
fi

if [[ ! -e ${config} ]]; then
    echo "Config file \"${config}\" doesn't exist."
    exit 1
fi

RANK=$PMI_RANK
if [[ -z $PMI_RANK ]]; then
    RANK=$SLURM_PROCID
fi

if [[ $RANK == 0 ]]; then
    echo "Using config file ${config}..."
    echo "Contents:"
    cat $config
    echo ""
fi
. $config

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$(realpath ../../dependencies/enc_wrapper-ins/lib/)"

if [[ ! $USE_STDOUT ]]; then
    ./out/enc_wrapper-benchmark ${MODE} ${RW} ${DIM0} ${DIM1} > ${OUTPUT_DIR}/enc_wrapper/$(basename ${config}).txt
else
    ./out/enc_wrapper-benchmark ${MODE} ${RW} ${DIM0} ${DIM1}
fi
