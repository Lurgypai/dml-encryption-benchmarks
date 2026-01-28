#!/bin/bash

config=$1

if [[ -z ${OUTPUT_DIR} ]]; then
    echo "Variable \"OUTPUT_DIR\" not set. Please set to specifiy where to output benchmark to."
    exit 1
fi

mkdir ${OUTPUT_DIR}/lifeboat
rm h5ex_d_rdwr_crypt.h5

if [[ ! -e ${config} ]]; then
    echo "Config file \"${config}\" doesn't exist."
    exit 1
fi

echo "Using config file ${config}..."
echo "Contents:"
cat $config
. $config

export LD_LIBRARY_PATH=${DEP_DIR}/gcrypt-ins/lib/
./out/lifeboat-flat ${MODE} ${DIM0} ${DIM1} > ${OUTPUT_DIR}/lifeboat/$(basename ${config})-out.txt

