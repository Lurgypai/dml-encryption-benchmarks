#!/bin/bash

if [[ ! -d dependencies ]]; then
    echo "Missing dependencies directory, exiting..."
    exit 1
fi

DEP_DIR=$(realpath dependencies)
LIBTOOL_NAME="libtool-2.5.4"

pushd dependencies
    wget https://ftpmirror.gnu.org/libtool/${LIBTOOL_NAME}.tar.gz
    tar -xvf ${LIBTOOL_NAME}.tar.gz
    rm ${LIBTOOL_NAME}.tar.gz
    pushd ${LIBTOOL_NAME}
        ./configure --prefix=${DEP_DIR}/libtool-ins
        make -j`nproc` && make install
    popd
popd
