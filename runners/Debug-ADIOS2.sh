#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=32
#SBATCH --cpus-per-task=1
#SBATCH --account=m2621
#SBATCH --time=00:30:00
#SBATCH --constraint=cpu
#SBATCH --qos=debug

# cd into benchmark dir
# run benchmark

GCRYPT_INS_DIR=$(realpath ../dependencies/gcrypt-ins/lib)

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:$GCRYPT_INS_DIR"

NODE_COUNT=1
PROCESS_COUNT=32

TAG="debug-ADIOS2-$(date "+%Y-%m-%d_%H:%M:%S")-${NODE_COUNT}n-${PROCESS_COUNT}p"

echo ${TAG};

pushd ../benchmarks >> /dev/null

srun --nodes=$NODE_COUNT --ntasks-per-node=$PROCESS_COUNT ./run_all.sh configs/write adios
srun --nodes=$NODE_COUNT --ntasks-per-node=$PROCESS_COUNT ./run_all.sh configs/read adios

pushd adios > /dev/null
    du -sh *.bp
    ./clean_outputs.sh
popd > /dev/null

popd > /dev/null

mv *.out "${TAG}.txt"
