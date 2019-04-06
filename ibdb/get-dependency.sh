ROOT_PATH=`pwd`
THIRD_LIBRARY="third-party"
THIRD_SRC="third-src"
DEPS_SOURCE=$ROOT_PATH/$THIRD_SRC
DEPS_PREFIX=$ROOT_PATH/$THIRD_LIBRARY
DEPS_CONFIG="--prefix=${DEPS_PREFIX} --disable-shared --with-pic"



export PATH=${DEPS_PREFIX}/lib:$PATH
export PATH=${DEPS_PREFIX}/include/boost:$PATH

mkdir -p log
mkdir -p ${DEPS_SOURCE} ${DEPS_PREFIX}
mkdir -p $DEPS_PREFIX/lib $DEPS_PREFIX/include
mkdir -p doc
mkdir -p ${THIRD_LIBRARY}
mkdir -p ${THIRD_SRC}
mkdir -p build

cd ${DEPS_SOURCE}

# boost
if [ -f "boost_succ" ]
then
    echo "boost is exist"
else
    echo "start install boost...."
    wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz >/dev/null
    tar -zxf boost_1_69_0.tar.gz >/dev/null
    cd boost_1_69_0
    cp -r boost ${DEPS_PREFIX}/include
    cd ..
    touch boost_succ
    echo "install boost done"
    cd ${DEPS_SOURCE}
fi


# cd $THIRD_SRC
# if [ -d "boost_1_69_0" ]; then
#     echo "boost is exist"
# else
#     wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz
#     tar -zxvf boost_1_69_0.tar.gz
#     cd boost_1_69_0
#     sh bootstrap.sh
#     ./b2 install
#     cp -r boost ${ROOT_PATH}/$THIRD_LIBRARY/include
#     cp -r stage/lib/. ${ROOT_PATH}/$THIRD_LIBRARY/lib/

# fi

if [ -d "gflags" ];then
    echo "gflags is exist"
else
    git clone --depth 1 https://github.com/gflags/gflags.git >/dev/null
    echo "cloning gflags"
    cd gflags
    mkdir -p builds
    cd builds
    cmake ..
    cp -r include/gflags ${DEPS_PREFIX}/include
    echo "install gflags done"
    cd ${DEPS_SOURCE}
fi

if [ -d "glog" ];then
    echo "glog is exist"
    # cd glog
    # ./autogen.sh && ./configure && make -j5 && make install
    # cd ${DEPS_SOURCE}
else
# v4.0
    git clone --depth 1 https://github.com/google/glog.git >/dev/null
    cd glog
    
    mkdir -p builds
    cd builds
    cmake ..
    cp -r glog ${DEPS_PREFIX}/include
    echo "install gflags done"
    cd ${DEPS_SOURCE}
    
fi



if [ -d "googletest" ];then
    echo "googletest is exist"
else
    git clone --depth 1 https://github.com/google/googletest.git
    cd googletest
    mkdir builds
    cd builds
    cmake ..
    make
    cp -r lib/. ${DEPS_PREFIX}/lib
    cd ${DEPS_SOURCE}
fi

if [ -d "googlemock" ];then
    echo "googlemock is exist"
else
    git clone --depth 1 https://github.com/google/googlemock.git
    cd googlemock
    cd googlemock
    cd include
    cp -r gmock ${DEPS_PREFIX}/include
    cd ${DEPS_SOURCE}
fi

if [ -d "double-conversion" ];then
    echo "double-conversion is exist"
else
    git clone --depth 1 https://github.com/google/double-conversion.git
    cd double-conversion
    cmake .
    cp -r double-conversion ${DEPS_PREFIX}/include
    cd ${DEPS_SOURCE}
fi
    
if [ -d "lz4" ];then
    echo "lz4 is exist"
else
    git clone --depth 1 https://github.com/lz4/lz4.git
    cd lz4
    make
    cd ${DEPS_PREFIX}/include
    # make install
    mkdir -p lz4
    cd ${DEPS_SOURCE}
    cd lz4
    cp lib/liblz4.dylib ${DEPS_PREFIX}/lib
    cp lib/liblz4.a ${DEPS_PREFIX}/lib
    cp lib/liblz4.pc.in ${DEPS_PREFIX}/lib
    cp lib/lz4frame.h ${DEPS_PREFIX}/include/lz4
    cp lib/lz4frame_static.h ${DEPS_PREFIX}/include/lz4
    cp lib/lz4hc.h ${DEPS_PREFIX}/include/lz4
    cp lib/lz4.h ${DEPS_PREFIX}/include/lz4
    cd ${DEPS_SOURCE}
fi

    
if [ -d "libunwind" ];then
    echo "libunwind is exist"
else
    git clone --depth 1 https://github.com/libunwind/libunwind.git
    cd ${DEPS_PREFIX}/include
    mkdir -p libunwind
    cd ${DEPS_SOURCE}
    cd libunwind
    cp -r include/. ${DEPS_PREFIX}/include/libunwind
    cd ${DEPS_SOURCE}
fi

if [ -d "libevent" ];then
    echo "libevent is exist"
else
    git clone --depth 1 https://github.com/libevent/libevent.git
    cd ${DEPS_PREFIX}/include
    mkdir -p libevent
    cd ${DEPS_SOURCE}
    cd libevent
    cp -r include/. ${DEPS_PREFIX}/include/libevent
    cd ${DEPS_SOURCE}
fi

if [ -d "openssl" ];then
    echo "openssl is exist"
else
    git clone --depth 1 https://github.com/openssl/openssl.git
    cd openssl
    mkdir -p ${DEPS_PREFIX}/openssl
    ./config --prefix=${DEPS_PREFIX}/openssl --openssldir=${DEPS_PREFIX}/openssl
    make -j5
    make install
    cp -r include/openssl ${DEPS_PREFIX}/include
    rm -rf ${DEPS_PREFIX}/lib/libssl.so*
    rm -rf ${DEPS_PREFIX}/lib/libcrypto.so*
    cd ${DEPS_PREFIX}/openssl
    cp -r lib/. ${DEPS_PREFIX}/lib
    cp -r bin/. ${DEPS_PREFIX}/bin
    cd ..
    rm -rf openssl
    cd ${DEPS_SOURCE}
fi


# if [ -d "libunwind" ];then
#     echo "libunwind is exist"
# else
#     git clone --depth 1 https://github.com/jemalloc/jemalloc.git
# fi

if [ -d "folly" ];then
    echo "folly is exist"
else
    git clone --depth 1 https://github.com/facebook/folly.git
    cd folly
    cp -r folly ${DEPS_PREFIX}/include
    cd ${DEPS_SOURCE}
fi

# 无法跨平台编译，因为有linux下的专用库
if [ -d "common" ];then
    echo "common is exist"
else
    git clone --depth 1 https://github.com/baidu/common.git
    cd ${DEPS_PREFIX}/include
    # mkdir -p common
    cd ${DEPS_SOURCE}
    cd common
    make -j5
    cp -r include/. ${DEPS_PREFIX}/include/common
    cd ${DEPS_SOURCE}
fi

#TODO 重新更改
if [ -d "protobuf2.6" ];then
    echo "protocol is exist"
    # cd protobuf2.6
    # ./autogen.sh
    # autoreconf -f -i -Wall,no-obsolete
    # export CPPFLAGS=-I${DEPS_PREFIX}/include
    # export LDFLAGS=-L${DEPS_PREFIX}/lib
    # ./configure $DEPS_CONFIG CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0"
    # make -j5
    # make install
    # cp -r src/.lib/. ${DEPS_PREFIX}/lib
    # cp -r src/.lib/protoc  ${DEPS_PREFIX}/bin
    # cp -r src/.lib/protoc  /usr/local/bin/protoc
    # cp -r src/.lib/. /usr/local/lib
    # cd ${DEPS_SOURCE}
else
    git clone -b v2.5.0 --depth 1 https://github.com/protocolbuffers/protobuf.git
    cd protobuf
    # ./autogen.sh
    autoreconf -f -i -Wall,no-obsolete
    export CPPFLAGS=-I${DEPS_PREFIX}/include
    export LDFLAGS=-L${DEPS_PREFIX}/lib
    ./configure $DEPS_CONFIG CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0"
    make -j5
    make install
    cd ${DEPS_SOURCE}
fi

if [ -d "leveldb" ];then
    echo "leveldb is exist"
    # cd leveldb
    # cp -r include/. ${DEPS_PREFIX}/include
    # mkdir -p build && cd build
    # cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
    # make install
    # cp -r include/. ${DEPS_PREFIX}/include/leveldb
    # cp libleveldb.a ${DEPS_PREFIX}/lib/libleveldb.a

    # cd ${DEPS_SOURCE}
else
    git clone --depth 1 https://github.com/google/leveldb.git
    # mkdir -p ${DEPS_PREFIX}/include/leveldb
    cd leveldb
    cp -r include/. ${DEPS_PREFIX}/include
    mkdir -p build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
    make install
    cp -r include/. ${DEPS_PREFIX}/include/leveldb
    cp libleveldb.a ${DEPS_PREFIX}/lib/libleveldb.a

    cd ${DEPS_SOURCE}
fi

if [ -d "brpc" ];then
    echo "brpc is exist"
else
    # git clone --depth 1 https://github.com/apache/incubator-brpc.git
    cd incubator-brpc
    sh config_brpc.sh --headers=${DEPS_PREFIX}/include --libs=${DEPS_PREFIX}/lib --with-glog
    make -j5
    cp -r output/bin/. ${DEPS_PREFIX}/bin
    cp -r output/lib/. ${DEPS_PREFIX}/lib
    mkdir -p ${DEPS_PREFIX}/include/brpc
    cp -r output/include/. ${DEPS_PREFIX}/include/brpc
    cd ..
    mv incubator-brpc brpc
    cd ${DEPS_SOURCE}
fi

if [ -d "zookeeper" ];then
    echo "zookeeper is exist"
else
    curl -L -O https://mirrors.tuna.tsinghua.edu.cn/apache/zookeeper/stable/zookeeper-3.4.14.tar.gz
    tar -zxvf zookeeper-3.4.14.tar.gz
    mv zookeeper-3.4.14 zookeeper
fi

cd $ROOT_PATH

# cp -r /usr/local/lib/. ${ROOT_PATH}/build
# cp -r /usr/local/lib ${DEPS_PREFIX}
# cp -r /usr/local/include ${DEPS_PREFIX}
# sh config_brpc.sh --headers=/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/third-party/include --libs=/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/third-party/lib

#          /Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/third-party/include