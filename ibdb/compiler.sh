ROOT_PATH=`pwd`
mkdir -p build
cd build
cmake ..
make -j5

cd $ROOT_PATH