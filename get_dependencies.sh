#!/bin/bash

rm -rf dependencies
mkdir dependencies
./get_autoconf.sh
./get_libtool.sh
./get_gcrypt.sh
./get_lifeboat.sh
./get_libsodium.sh
./get_adios.sh
