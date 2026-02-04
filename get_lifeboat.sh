#!/bin/bash

if [[ ! -d dependencies ]]; then
    echo "Missing dependencies dir, exiting..."
    exit 1
fi

DEP_DIR=$(realpath dependencies)
export HDF5_LIBTOOL=${DEP_DIR}/libtool-ins/bin/libtool
export HDF5_AUTOCONF=${DEP_DIR}/autoconf-ins/bin/autoconf
export HDF5_AUTOHEADER=${DEP_DIR}/autoconf-ins/bin/autoheader

pushd dependencies > /dev/null
    git clone git@github.com:LifeboatLLC/HDF5-Encryption.git
    pushd HDF5-Encryption/hdf5/hdf5-1_14_3
        export LIBS="-lgcrypt"
        export CFLAGS="-g -O0 -I${DEP_DIR}/gcrypt-ins/include -I${DEP_DIR}/gpgerror-ins/include"
        export LDFLAGS="-lgcrypt -L${DEP_DIR}/gcrypt-ins/lib"
        export LD_LIBRARY_PATH=${DEP_DIR}/gcrypt-ins/lib/
        ./autogen.sh
        ./configure --prefix=${DEP_DIR}/lifeboat-ins
        make -j`nproc` && make install
    popd > /dev/null
popd
