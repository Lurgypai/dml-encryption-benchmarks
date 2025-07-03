#!/bin/bash

pushd HDF5-Encryption/hdf5/hdf5-1_14_3

export LIBS="-lgcrypt"

./autogen.sh
./configure --prefix="/workspace/lifeboat-hdf5-ins"
make -j8 && make install

popd
