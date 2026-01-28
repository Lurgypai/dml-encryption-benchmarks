#!/bin/bash

rm -rf dependencies
mkdir dependencies
./get_libtool.sh
./get_libgcrypt.sh
./get_lifeboat.sh
./get_libsodium.sh
./get_adios.sh
