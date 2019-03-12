ROOT_PATH=`pwd`

PROTOC_PATH=$ROOT_PATH/src/protoc

protoc --proto_path=$PROTOC_PATH --cpp_out=$PROTOC_PATH storage.proto