ROOT_PATH=`pwd`
echo run this $1 file

cp -r third-party/lib/. build
cd build

./$1

cd $ROOT_PATH


