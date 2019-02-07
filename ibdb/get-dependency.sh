ROOT_PATH=`pwd`
THIRD_LIBRARY="third-party"
THIRD_SRC="third-src"
DEPS_SOURCE=$ROOT_PATH/$THIRD_SRC
DEPS_PREFIX=$ROOT_PATH/$THIRD_LIBRARY
DEPS_CONFIG="--prefix=${DEPS_PREFIX} --disable-shared --with-pic"



export PATH=${DEPS_PREFIX}/lib:$PATH
export PATH=${DEPS_PREFIX}/include/boost:$PATH

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
    mv boost ${DEPS_PREFIX}/include
    cd ..
    touch boost_succ
    echo "install boost done"
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
    git clone https://github.com/gflags/gflags.git >/dev/null
    echo "cloning gflags"
    cd gflags
    mkdir -p builds
    cd builds
    cmake ..
    mv include/gflags ${DEPS_PREFIX}/include
    echo "install gflags done"
    cd ${DEPS_SOURCE}
fi

if [ -d "glog" ];then
    echo "glog is exist"
else
    git clone https://github.com/google/glog.git >/dev/null
    echo "cloning glog"
    cd glog
    mkdir -p builds
    cd builds
    cmake ..
    mv glog ${DEPS_PREFIX}/include
    echo "install gflags done"
    cd ${DEPS_SOURCE}
fi


# if [ -d ""];then
#     git clone https://github.com/google/double-conversion.git
#     cd double-conversion
#     cmake -DBUILD_SHARED_LIBS=ON .
#     make

# if [ -d "folly" ]; then
#     wget https://github.com/google/googletest/archive/release-1.8.0.tar.gz
#     tar -zxvf boost_1_69_0.tar.gz
# fi

cd $ROOT_PATH