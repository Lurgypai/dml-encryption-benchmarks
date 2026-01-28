#!/bin/bash

DEP_DIR=$(realpath dependencies)

if [[ ! -d ${DEP_DIR} ]]; then
    echo "Missing \"dependencies\" directory, exiting..."
    exit 1
fi

pushd dependencies
    git clone git@github.com:pmodels/mpich.git
    pushd mpich
        git submodule update --init
        ./autogen.sh
        ./configure --prefix=${DEP_DIR}/mpich-ins
        make -j`nproc` && make install
    popd
popd
