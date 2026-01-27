#!/bin/bash

if [[ ! -d dependencies ]]; then
    echo "Missing dependency dir, exiting..."
    exit 1
fi

DEP_DIR=$(realpath dependencies)
SODIUM_NAME="libsodium-1.0.21"

pushd dependencies > /dev/null
    wget https://download.libsodium.org/libsodium/releases/${SODIUM_NAME}.tar.gz
    tar -xvf ${SODIUM_NAME}.tar.gz
    rm ${SODIUM_NAME}.tar.gz
    pushd ${SODIUM_NAME} > /dev/null
        ./configure --prefix=${DEP_DIR}/libsodium-ins
        make -j`nproc` && make install
    popd > /dev/null
popd > /dev/null
