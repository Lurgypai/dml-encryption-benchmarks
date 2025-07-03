#!/bin/bash

rm -r out
mkdir out

pushd out

CC=/workspace/lifeboat-hdf5-ins/bin/h5cc cmake ..

popd
