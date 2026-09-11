#!/bin/bash

RANK=$PMI_RANK
if [[ -z $PMI_RANK ]]; then
    RANK=$SLURM_PROCID
fi

export OUTPUT_DIR=$(realpath output)

if [[ ! -d ${1} ]]; then
    if [[ $RANK == 0 ]]; then
        echo "\"${1}\" is not a directory."
    fi
    exit 1
fi

CONFIG_DIR=$(realpath ${1})

LIBRARY=${2}

if [[ ${LIBRARY} == "lifeboat" ]]; then
    pushd lifeboat > /dev/null
    for c in ${CONFIG_DIR}/*; do
        if [[ $RANK == 0 ]]; then
          echo "running lifeboat benchmark on config $c"
        fi
      ./run_lifeboat.sh $c
    done
    popd > /dev/null
elif [[ ${LIBRARY} == "adios" ]]; then
    pushd adios > /dev/null
    for c in ${CONFIG_DIR}/*; do
        if [[ $RANK == 0 ]]; then
            echo "running adios benchmark on config $c"
        fi
            ./run_adios.sh $c
    done
    popd > /dev/null
else
    if [[ $RANK == 0 ]]; then
        echo "Invalid library \"${LIBRARY}\""
    fi
    exit 1
fi



