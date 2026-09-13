#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
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
PROCESS_COUNT=1

TAG="debug-Lifeboat-$(date "+%Y-%m-%d_%H:%M:%S")-${NODE_COUNT}n-${PROCESS_COUNT}p"

pushd ../benchmarks >> /dev/null

srun --nodes=$NODE_COUNT --ntasks-per-node=$PROCESS_COUNT ./run_all.sh configs/write lifeboat
srun --nodes=$NODE_COUNT --ntasks-per-node=$PROCESS_COUNT ./run_all.sh configs/read lifeboat

pushd lifeboat
    du -sh *.h5
    ./clean_outputs.sh
popd

popd > /dev/null

mv *.out "${TAG}.txt"
