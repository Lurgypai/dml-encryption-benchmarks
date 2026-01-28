#!/bin/bash

rm -r out
mkdir out

COMPILER=""
CXX_COMPILER=""
if [[ -z $(which ${COMPILER}) ]]; then
    COMPILER=$(realpath "../../dependencies/mpich-ins/bin/mpicc")
    CXX_COMPILER=$(realpath "../../dependencies/mpich-ins/bin/mpicxx")
    if [[ ! -f ${COMPILER} ]]; then
        echo "Unable to find mpicc, exiting..."
        exit 1
    fi
fi

pushd out

CC=${COMPILER} CXX=${CXX_COMPILER} cmake ..

popd
