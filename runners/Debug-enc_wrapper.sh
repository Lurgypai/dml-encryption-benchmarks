#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=16
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

node_counts="1"
process_counts="16"

mode="debug"

for NODE_COUNT in ${node_counts}; do
    for PROCESS_COUNT in ${process_counts}; do

        TAG="${mode}-enc_wrapper-${DATE_TIME}-${NODE_COUNT}n-${PROCESS_COUNT}p"

        echo ${TAG};

        pushd ../benchmarks > /dev/null
            rm -r output
            mkdir output
            # relative to the dir where the final run script is
            export OUTPUT_DIR="../output"

            ls

            time srun --nodes=$NODE_COUNT --ntasks-per-node=$PROCESS_COUNT ./run_all.sh configs/write enc_wrapper
            time srun --nodes=$NODE_COUNT --ntasks-per-node=$PROCESS_COUNT ./run_all.sh configs/read enc_wrapper

            pushd enc_wrapper > /dev/null
                du -sh *.store
                ./clean_outputs.sh
            popd > /dev/null

            mv output ../runners/${TAG}
        popd > /dev/null
    done
done


mv *.out "${mode}-enc_wrapper-${DATE_TIME}.txt"
