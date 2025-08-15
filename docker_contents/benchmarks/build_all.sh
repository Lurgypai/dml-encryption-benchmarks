#!/bin/bash

# lifeboat benchmarks
pushd lifeboat

./out.sh
pushd out
make -j8
popd

popd

# adios benchmarks
pushd adios
./out.sh
pushd out
make -j8
popd
popd
