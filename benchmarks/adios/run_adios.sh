#!/bin/bash

config=$1

USE_STDOUT=""

RANK=$PMI_RANK
if [[ -z $PMI_RANK ]]; then
    RANK=$SLURM_PROCID
fi

if [[ -z ${OUTPUT_DIR} ]]; then
    echo "Variable \"OUTPUT_DIR\" not set, printing to stdout"
    USE_STDOUT="yes"
fi

if [[ ! $USE_STDOUT ]]; then
    if [[ ! -e ${OUTPUT_DIR} ]]; then
        if [[ $RANK == 0 ]]; then
            echo "missing output dir"
            exit -1
        fi
    fi
fi

if [[ ! -e ${config} ]]; then
    echo "Config file \"${config}\" doesn't exist."
    exit 1
fi

# if [[ $RANK == 0 ]]; then
    # echo "Using config file ${config}..."
    # echo "Contents:"
    # cat $config
    # echo ""
# fi
. $config

if [[ ! $USE_STDOUT ]]; then
    OUT_FILE=${OUTPUT_DIR}/$(basename $config).txt
    ./out/adios-benchmark ${MODE} ${RW} ${DIM0} ${DIM1} > $OUT_FILE
    enc_times_dir=${OUTPUT_DIR}/$(basename $config)-enc_times
    mkdir $enc_times_dir
    mv *-enc_times.txt $enc_times_dir
else
    ./out/adios-benchmark ${MODE} ${RW} ${DIM0} ${DIM1}
    echo "Enc Times:"
    for file in *"-enc_times.txt"; do
        echo -n "$file, "
        cat $file
    done
fi
