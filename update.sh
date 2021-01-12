#!/bin/sh

cd kcall_static
make all
cd ..
cd modules
./makeall
