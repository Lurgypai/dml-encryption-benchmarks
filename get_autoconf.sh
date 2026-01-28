#!/bin/bash

DEP_DIR=$(realpath dependencies)
AUTOCONF_NAME="autoconf-2.72"

pushd dependencies
    wget https://ftp.gnu.org/gnu/autoconf/${AUTOCONF_NAME}.tar.gz
    tar -xvf ${AUTOCONF_NAME}.tar.gz
    rm ${AUTOCONF_NAME}.tar.gz

    pushd ${AUTOCONF_NAME}/
        ./configure --prefix=${DEP_DIR}/autoconf-ins
        make -j`nproc` && make install
    popd
popd
