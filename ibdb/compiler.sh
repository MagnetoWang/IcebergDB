#!/usr/bin/env bash
ROOT_PATH=`pwd`

#echo `sh generate-code.sh` 2>/dev/null

#sh generate-code.sh 2>/dev/null
mkdir -p build
cd build
# cmake -Wdeprecated-declarations=ON -Wno-deprecated=ON ..
#echo ` -Wdeprecated-declarations` 2>/dev/null
cmake ..
make -j5 -Wdeprecated-declarations

cd $ROOT_PATH