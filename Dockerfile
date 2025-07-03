FROM ubuntu:22.04

WORKDIR /workspace

RUN apt update && apt install -y \
    build-essential \
    make \
    git \
    automake \
    autoconf \
    libtool \
    libtool-bin \
    wget \
    lsb-release \
    software-properties-common \
    gnupg \
    libgcrypt-dev \
    cmake \
    vim \
    pkg-config \
    && apt clean

RUN git clone https://github.com/LifeboatLLC/HDF5-Encryption.git

COPY ./docker_contents .

RUN ./build_lifeboat.sh

WORKDIR /workspace/benchmarks

RUN ./build_all.sh

WORKDIR /workspace
