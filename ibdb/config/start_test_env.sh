#!/usr/bin/env bash
ROOT_PATH=`pwd`


cd ..
cd third-src/zookeeper/

cp $ROOT_PATH/zoo.cfg conf/zoo.cfg

cd bin/
sh zkServer.sh start


./ibdbs --start_program="StartNode" --start_endpoint="127.0.0.1:1990" --start_zk_endpoint="127.0.0.1:2181"

./ibdbs --start_program="StartNode" --start_endpoint="127.0.0.1:1991" --start_zk_endpoint="127.0.0.1:2181"