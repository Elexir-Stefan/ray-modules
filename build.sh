#!/bin/sh

cd kcall_static
./mkmake
make rebuild
if [ $? != 0 ]; then
    exit -1
fi

cd ..
cd modules
./makeall refresh
