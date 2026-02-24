#!/bin/bash

config=$1

if [[ -z ${OUTPUT_DIR} ]]; then
    echo "Variable \"OUTPUT_DIR\" not set, printing locally"
else
    echo "Using ${OUTPUT_DIR} as output dir"
fi

if [[ -d ${OUTPUT_DIR} ]]; then
    mkdir ${OUTPUT_DIR}/lifeboat
fi

rm h5ex_d_rdwr_crypt.h5

if [[ ! -e ${config} ]]; then
    echo "Config file \"${config}\" doesn't exist."
    exit 1
fi

echo "Using config file ${config}..."
echo "Contents:"
cat $config
. $config

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH};${DEP_DIR}/gcrypt-ins/lib/;${DEP_DIR}/lifeboat-ins/lib/"
if [[ -d ${OUTPUT_DIR} ]]; then
    ./out/lifeboat-flat ${MODE} write ${DIM0} ${DIM1} > ${OUTPUT_DIR}/lifeboat/$(basename ${config})-write.txt
    ./out/lifeboat-flat ${MODE} read ${DIM0} ${DIM1} > ${OUTPUT_DIR}/lifeboat/$(basename ${config})-read.txt
else
    ./out/lifeboat-flat ${MODE} write ${DIM0} ${DIM1}
    ./out/lifeboat-flat ${MODE} read ${DIM0} ${DIM1}
fi

