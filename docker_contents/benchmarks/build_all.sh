#!/bin/bash

# lifeboat benchmarks
pushd lifeboat

./out.sh
pushd out
make -j8
popd

popd
