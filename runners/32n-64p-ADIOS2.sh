#!/bin/bash

#SBATCH --nodes=32
#SBATCH --ntasks-per-node=64
#SBATCH --cpus-per-task=1
#SBATCH --account=m2621
#SBATCH --time=02:00:00
#SBATCH --constraint=cpu
#SBATCH --qos=regular


# just for filenames 
mode="32n-64p"

GCRYPT_INS_DIR=$(realpath ../dependencies/gcrypt-ins/lib)

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:$GCRYPT_INS_DIR"

DATE_TIME="$(date "+%Y-%m-%d_%H:%M:%S")"

node_counts="32"
process_counts="64"

for NODE_COUNT in ${node_counts}; do
    for PROCESS_COUNT in ${process_counts}; do

        TAG="${mode}-ADIOS2-${DATE_TIME}-${NODE_COUNT}n-${PROCESS_COUNT}p"

        echo ${TAG};

        pushd ../benchmarks > /dev/null
            rm -r output
            mkdir output
            # relative to the dir where the final run script is
            export OUTPUT_DIR="../output"

            ls

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


mv *.out "${mode}-adios2-${DATE_TIME}.txt"
