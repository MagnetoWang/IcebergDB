ROOT_PATH=`pwd`

sh generate-code.sh


mkdir -p build
cd build
cmake ..
make -j5

cd $ROOT_PATH