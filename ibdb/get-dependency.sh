ROOT_PATH=`pwd`
THIRD_LIBRARY="third-party"
THIRD_SRC="third-src"
DEPS_SOURCE=$ROOT_PATH/$THIRD_SRC
DEPS_PREFIX=$ROOT_PATH/$THIRD_LIBRARY
DEPS_CONFIG="--prefix=${DEPS_PREFIX} --disable-shared --with-pic"



export PATH=${DEPS_PREFIX}/bin:$PATH

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
    echo "boost exist"
else
    echo "start install boost...."
    # wget http://pkg.4paradigm.com/rtidb/dev/boost-header-only.tar.gz >/dev/null
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

# if [ -d "folly" ]; then
#     wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz
#     tar -zxvf boost_1_69_0.tar.gz
# fi

cd $ROOT_PATH