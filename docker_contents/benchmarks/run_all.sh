#!/bin/bash

# lifeboat
for c in configs/*; do
    echo "running lifeboat benchmark on config $c"
    lifeboat/run_lifeboat.sh c
done


# adios 
for c in configs/*; do
    echo "running adios benchmark on config $c"
    adios/run_adios.sh c
done
