ROOT_PATH=`pwd`
echo run this $1 file

# cp -r third-party/lib/. build
cd build

if [ $1 = 'all' ]; then
        rows=$(ls | grep test | awk '{print $1}')
        echo $rows
        for test in $rows
        do
            ./$test
        done
    else
        ./$1
fi

# for i in {1..2}
# do

# done

cd $ROOT_PATH


