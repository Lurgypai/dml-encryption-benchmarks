#!/bin/bash

tar -xvf darshan-3.4.7.tar.gz

rm darshan-3.4.7.tar.gz

pushd darshan-3.4.7
./prepare.sh

pushd darshan-runtime
./configure --with-log-path=/workspace/darshan-logs --with-jobid-env=PBS_JOBID CC=gcc
make
make install
mkdir /workspace/darshan-logs
chmod +x darshan-mk-log-dirs.pl
./darshan-mk-log-dirs.pl
popd

pushd darshan-util
./configure
make
make install
ldconfig
apt install -y \
    libpod-latex-perl \
    texlive-latex-base \
    texlive-font-utils \
    gnuplot

cpan HTML::Entities 
popd

popd
