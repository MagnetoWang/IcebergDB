#!/usr/bin/env bash
ROOT_PATH=`pwd`


cd ..
cd third-src/zookeeper/

cp $ROOT_PATH/zoo.cfg conf/zoo.cfg

cd bin/
sh zkServer.sh start