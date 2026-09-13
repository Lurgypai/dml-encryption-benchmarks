#!/bin/bash

#SBATCH --nodes=64
#SBATCH --ntasks-per-node=64
#SBATCH --cpus-per-task=1
#SBATCH --account=m2621
#SBATCH --time=00:30:00
#SBATCH --constraint=cpu
#SBATCH --qos=debug

# cd into benchmark dir
# run benchmark

GCRYPT_INS_DIR=$(realpath ../dependencies/gcrypt-ins/lib)

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:$GCRYPT_INS_DIR"

DATE_TIME="$(date "+%Y-%m-%d_%H:%M:%S")"

for NODE_COUNT in "8 16 32 64"; do
    for PROCESS_COUNT in "64"; do

        TAG="all-ADIOS2-${DATE_TIME}-${NODE_COUNT}n-${PROCESS_COUNT}p"

        echo ${TAG};

        pushd ../benchmarks >> /dev/null
            mkdir output
            export OUTPUT_DIR="output"

            srun --nodes=$NODE_COUNT --ntasks-per-node=$PROCESS_COUNT ./run_all.sh configs/write adios
            srun --nodes=$NODE_COUNT --ntasks-per-node=$PROCESS_COUNT ./run_all.sh configs/read adios

            pushd adios > /dev/null
                du -sh *.bp
                ./clean_outputs.sh
            popd > /dev/null

            mv output ../runners/${TAG}
        popd > /dev/null
    done
done


mv *.out "All-adios2-${DATE_TIME}.txt"
