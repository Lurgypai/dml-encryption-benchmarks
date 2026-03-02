#!/bin/bash

echo "Removing old configs dir"
rm -r configs
echo "Generating new configs dirs"
mkdir configs
mkdir configs/read
mkdir configs/write

echo "Generating configs"
python3 generate_configs.py

echo "Complete."
