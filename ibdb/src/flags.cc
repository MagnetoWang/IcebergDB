#include <gflags/gflags.h>
// Test GFlags
DEFINE_string(test_flags, "test test test", "config the ip and port that rtidb serves for");

//Test GLog
DEFINE_string(ibdb_log_dir, "/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/", "config log dir for testing");
DEFINE_string(log_dir_info, "/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/info", "config log dir for testing");
DEFINE_string(log_dir_warn, "/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/warn", "config log dir for testing");
DEFINE_string(log_dir_error, "/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/error", "config log dir for testing");
DEFINE_string(log_dir_fatal, "/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/fatal", "config log dir for testing");


//dir
// DEFINE_string


//log
DEFINE_uint64(limiter_max_required, 100, "limiter_max_required in log/format.h");
// TODO 可以设置为1，测试test
DEFINE_uint32(log_index_sparse_threshold, 100, "insert offset-position to map every 100 offset");

//table dir
DEFINE_string(db_root, "ibdb/db", "config db's log which store key and value");

//segment
DEFINE_uint32(skiplist_height, 4, "config segment skiplist");
DEFINE_string(table_timestamp, "timestamp", "every table has timestamp as a key");

//rpc
DEFINE_int32(timeout_ms, 100, "rpc client timeout in milliseconds");
DEFINE_int32(max_retry, 5, "Max retries(not including the first RPC)");



