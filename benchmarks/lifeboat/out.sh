#!/bin/bash

rm -r out
mkdir out

DEP_DIR=$(realpath ../../dependencies)

pushd out

CC=${DEP_DIR}/lifeboat-ins/bin/h5cc cmake ..

popd
