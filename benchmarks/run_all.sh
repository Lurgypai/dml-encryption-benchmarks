#!/bin/bash

export OUTPUT_DIR=$(realpath output)
rm -r ${OUTPUT_DIR}
mkdir ${OUTPUT_DIR}
lfs setstripe -c 8 -S $((16384*16384)) ${OUTPUT_DIR}

CONFIG_DIR=$(realpath configs)

# lifeboat
# pushd lifeboat
# for c in ${CONFIG_DIR}/*; do
    # echo "running lifeboat benchmark on config $c"
    # ./run_lifeboat.sh $c
# done
# popd


# adios 
pushd adios
for c in ${CONFIG_DIR}/*; do
    echo "running adios benchmark on config $c"
    ./run_adios.sh $c
done
popd
