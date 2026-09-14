#!/bin/bash

if [[ ! -d dependencies ]]; then
    echo "Missing dependencies directory, exiting..."
    exit 1
fi

DEP_DIR=$(realpath dependencies)

pushd dependencies
    rm -rf ADIOS2
    git clone git@github.com:Lurgypai/ADIOS2.git
    pushd ADIOS2
        mkdir out
        pushd out
            cmake .. \
                -DCMAKE_PREFIX_PATH=${DEP_DIR}/libsodium-ins \
                -DCMAKE_INSTALL_PREFIX="${DEP_DIR}/adios-ins"
            make -j`nproc`
            make install
        popd
    popd
popd
