#!/bin/bash

config=$1

if [[ -z ${OUTPUT_DIR} ]]; then
    echo "Variable \"OUTPUT_DIR\" not set, printing locally"
else
    echo "Using ${OUTPUT_DIR} as output dir"
fi

if [[ -n ${OUTPUT_DIR} ]]; then
    mkdir -p "${OUTPUT_DIR}/lifeboat"
fi

if [[ ! -e ${config} ]]; then
    echo "Config file \"${config}\" doesn't exist."
    exit 1
fi

echo "Using config file ${config}..."
echo "Contents:"
cat $config
. $config

algs="0 1 2"
modes="0 1"

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${DEP_DIR}/gcrypt-ins/lib/:${DEP_DIR}/lifeboat-ins/lib/"

if [[ "${MODE}" = "def" ]]; then
    if [[ -d ${OUTPUT_DIR} ]]; then
        ./out/lifeboat-flat ${MODE} ${RW} 0 0 ${DIM0} ${DIM1} > ${OUTPUT_DIR}/lifeboat/$(basename ${config}).txt
    else
        ./out/lifeboat-flat ${MODE} ${RW} 0 0 ${DIM0} ${DIM1}
    fi
else
    for alg in ${algs}; do
        for mode in ${modes}; do
            if [[ -d ${OUTPUT_DIR} ]]; then
                ./out/lifeboat-flat ${MODE} ${RW} ${alg} ${mode} ${DIM0} ${DIM1} > ${OUTPUT_DIR}/lifeboat/${alg}-${mode}-$(basename ${config}).txt
            else
                ./out/lifeboat-flat ${MODE} ${RW} ${alg} ${mode} ${DIM0} ${DIM1}
            fi
        done
    done
fi
