#!/usr/bin/env bash
ROOT_PATH=`pwd`
THIRD_LIBRARY="third-party"
THIRD_SRC="third-src"
DEPS_SOURCE=$ROOT_PATH/$THIRD_SRC
DEPS_PREFIX=$ROOT_PATH/$THIRD_LIBRARY
DEPS_CONFIG="--prefix=${DEPS_PREFIX} --disable-shared --with-pic"
# version control
PROTOBUF_VERSION=2.6

cd third-src/protobuf$PROTOBUF_VERSION
mkdir -p include
mkdir -p lib
export CPPFLAGS=-I${ROOT_PATH}/third-src/protobuf$PROTOBUF_VERSION/include
export LDFLAGS=-L${ROOT_PATH}/third-src/protobuf$PROTOBUF_VERSION/lib
./configure $DEPS_CONFIG CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0"
# ./configure --prefix=${ROOT_PATH}/third-src/protobuf$PROTOBUF_VERSION/include --disable-shared --with-pic
# make -j5
make install
# cp -r .libs lib
cp -r src/lib/. ${DEPS_PREFIX}/lib
cp -r src/lib/protoc  ${DEPS_PREFIX}/bin
cp -r src/lib/protoc  /usr/local/bin/protoc
cp -r src/lib/. /usr/local/lib
