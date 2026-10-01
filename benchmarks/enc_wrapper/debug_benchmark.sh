#!/bin/bash

config=$1

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

if [[ ! -z ${SLURM_PROCID} ]]; then
    echo "Using SLURM_PROCID as rank..."
    RANK=${SLURM_PROCID}
elif [[ ! -z ${PMI_RANK} ]]; then
    echo "Using PMI_RANK as rank..."
    RANK=${PMI_RANK}
else
    echo "Not run with mpi, assigning output log rank to 0"
    RANK="0"
fi

output="termout/${RANK}"

date +%Y-%m-%d_%H-%M-%S >> $output
echo "---- (run_benchmark.sh) Running benchmark below ----" >> $output
# out/benchmark ${1} ${2} >> $output 2>&1
gdb -ex "set confirm off" -ex "run" -ex "bt full" -ex "quit" --args ./out/enc_wrapper-benchmark ${MODE} ${RW} ${DIM0} ${DIM1} >> $output 2>&1
echo "---- (run_benchmark.sh) completed benchmark ----" >> $output
