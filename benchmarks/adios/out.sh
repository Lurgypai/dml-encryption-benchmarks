#!/bin/bash

rm -r out
mkdir out

COMPILER="mpicc"
CXX_COMPILER="mpicxx"
if [[ -z $(which ${COMPILER}) ]]; then
    echo "Using local mpicc"
    COMPILER=$(realpath "../../dependencies/mpich-ins/bin/mpicc")
    CXX_COMPILER=$(realpath "../../dependencies/mpich-ins/bin/mpicxx")
    if [[ ! -f ${COMPILER} ]]; then
        echo "Unable to find mpicc, exiting..."
        exit 1
    fi
else
    echo "Using system mpicc"
fi

pushd out

CC=${COMPILER} CXX=${CXX_COMPILER} cmake ..

popd
