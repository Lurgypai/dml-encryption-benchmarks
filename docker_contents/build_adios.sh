#!/bin/bash

git clone https://github.com/ornladios/ADIOS2.git

pushd ADIOS2
mkdir out

pushd out
cmake .. -DCMAKE_INSTALL_PREFIX="/workspace/adios-ins"
make -j8
make install
popd

popd
