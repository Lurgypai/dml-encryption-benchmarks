#1/bin/bash

pushd out
    bear -- make -j`nproc`
    mv compile_commands.json ..
popd
