ROOT_PATH=`pwd`

echo sh generate-code.sh 2>/dev/null


mkdir -p build
cd build
# cmake -Wdeprecated-declarations=ON -Wno-deprecated=ON ..
cmake ..
make -j5

cd $ROOT_PATH