#!/bin/bash

if [[ ! -d dependencies ]]; then
    echo "Missing dependencies directory, exiting..."
    exit 1
fi

DEP_DIR=$(realpath dependencies)
WRAPPER_OUT_DIR="${DEP_DIR}/enc_wrapper-ins"
IO_OUT_DIR="${DEP_DIR}/enc_io-ins"
MPI_DIR="${DEP_DIR}/mpich-ins"

export GCRYPT_ROOT_DIR=${DEP_DIR}/gcrypt-ins
export GPG_ERROR_ROOT_DIR=${DEP_DIR}/gpgerror-ins


pushd dependencies
    rm -rf enc_wrapper
    git clone https://github.com/Lurgypai/enc_wrapper.git
    pushd enc_wrapper
        git switch AddIndex
        pushd wrapper
            mkdir out
            pushd out
                cmake .. \
                    -DCMAKE_EXPORT_COMPILE_COMMANDS=On \
                    -DCMAKE_INSTALL_PREFIX=${WRAPPER_OUT_DIR} \
                    -DENC_WRAPPER_ENABLE_NETTLE=Off
                make -j`nproc` && make install
            popd
        popd
        pushd io
            echo "MPI enabled"
            MPICC=$(which mpicc)
            MPICXX=$(which mpicxx)
            if [[ -z $MPICC || -z $MPICC ]]; then
                MPICC=${MPI_DIR}/bin/mpicc
                MPICXX=${MPI_DIR}/bin/mpicxx

                if [[ ! -f $MPICC || ! -f $MPICXX ]]; then
                    echo "MPI was not detected on the system, and not found in the $DEP_DIR"
                    exit 1
                else
                    echo "Using MPI in ${DEP_DIR}, MPICC=$MPICC MPICXX=$MPICXX"
                fi
            else
                echo "Using system MPI, MPICC=$MPICC, MPICXX=$MPICXX"
            fi

            CC=$MPICC
            CXX=$MPICXX
            CFLAGS="-DENABLE_MPI"

            mkdir out
            pushd out
                cmake .. \
                    -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX} -DCMAKE_C_FLAGS=${CFLAGS} \
                    -Denc_wrapper_DIR=${WRAPPER_OUT_DIR}/cmake \
                    -DCMAKE_INSTALL_PREFIX=${IO_OUT_DIR} \
                    -DCMAKE_BUILD_TYPE=Debug
                make -j`nproc` && make install
            popd
        popd
    popd
popd
