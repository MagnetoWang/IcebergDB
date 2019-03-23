ROOT_PATH=`pwd`
echo run this $1 file

# cp -r third-party/lib/. build
cd build
mkdir -p reports
if [ $1 = 'all' ]; then
        rows=$(ls | grep test | awk '{print $1}')
        echo $rows
        for test in $rows
        do
            ./$test --gtest_output=xml:./reports/$test.xml
        done
    else
        ./$1 --gtest_output=xml:./reports/$1.xml
fi

# for i in {1..2}
# do

# done

cd $ROOT_PATH


