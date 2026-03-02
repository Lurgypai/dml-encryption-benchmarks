#!/bin/bash

export OUTPUT_DIR=$(realpath output)
rm -r ${OUTPUT_DIR}
mkdir ${OUTPUT_DIR}
# lfs setstripe -c 8 -S $((16384*16384)) ${OUTPUT_DIR}

if [[ ! -d ${1} ]]; then
    echo "\"${1}\" is not a directory."
    exit 1
fi

CONFIG_DIR=$(realpath ${1})

LIBRARY=${2}

if [[ ${LIBRARY} == "lifeboat" ]]; then
    pushd lifeboat
    for c in ${CONFIG_DIR}/*; do
      echo "running lifeboat benchmark on config $c"
      ./run_lifeboat.sh $c
    done
    popd
elif [[ ${LIBRARY} == "adios" ]]; then
    pushd adios
    for c in ${CONFIG_DIR}/*; do
        echo "running adios benchmark on config $c"
        ./run_adios.sh $c
    done
    popd
else
    echo "Invalid library \"${LIBRARY}\""
    exit 1
fi



