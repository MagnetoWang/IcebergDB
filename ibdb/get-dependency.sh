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
else
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
    cp -r include/openssl ${DEPS_PREFIX}/include
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

if [ -d "protobuf"];then
    echo "protocol is exist"
else
    git clone -b v2.6.0 --depth 1 https://github.com/protocolbuffers/protobuf.git
    cd protobuf
    ./autogen.sh
    export CPPFLAGS=-I${DEPS_PREFIX}/include
    export LDFLAGS=-L${DEPS_PREFIX}/lib
    ./configure $DEPS_CONFIG CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0"
    make -j5
    make install
    cd ${DEPS_SOURCE}
fi

cd $ROOT_PATH

# cp -r /usr/local/lib/. ${ROOT_PATH}/build
# cp -r /usr/local/lib ${DEPS_PREFIX}
# cp -r /usr/local/include ${DEPS_PREFIX}

# /Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/third-party/include