#!/bin/bash

INS_DIR=$(realpath ../../dependencies)
WRAPPER_DIR="${INS_DIR}/enc_wrapper-ins/cmake"
IO_DIR="${INS_DIR}/enc_io-ins/cmake"

if [[ ! -d ${WRAPPER_DIR} ]] || [[ ! -d ${IO_DIR} ]]; then
    echo "ERROR: Missing enc_io or enc_wrapper cmake dirs"
    exit 1
fi

rm -r out
mkdir out
cd out
CC=mpicc CXX=mpicxx cmake .. \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=On \
    -Denc_wrapper_DIR="${INS_DIR}/enc_wrapper-ins/cmake" \
    -Denc_io_DIR="${INS_DIR}/enc_io-ins/cmake"
