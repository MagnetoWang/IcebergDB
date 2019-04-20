#!/usr/bin/env bash
ROOT_PATH=`pwd`

PROTOC_PATH=$ROOT_PATH/src/protobuf

third-party/bin/protoc --proto_path=$PROTOC_PATH --cpp_out=$PROTOC_PATH $PROTOC_PATH/storage.proto -Wdeprecated-declarations
third-party/bin/protoc --proto_path=$PROTOC_PATH --cpp_out=$PROTOC_PATH $PROTOC_PATH/rpc.proto -Wdeprecated-declarations
# third-party/bin/protoc --version

# be careful protoc must be v2.6.0 or lower version
# protoc --proto_path=$PROTOC_PATH --cpp_out=$PROTOC_PATH storage.proto

cd $ROOT_PATH

# third-party/bin/protoc --proto_path=/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/src/protobuf --cpp_out=/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/src/protobuf storage.proto

