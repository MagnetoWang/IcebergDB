#include <gflags/gflags.h>
// Test GFlags
DEFINE_string(test_flags, "test test test", "config the ip and port that rtidb serves for");

//Test GLog
DEFINE_string(ibdb_log_dir, "/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/", "config log dir for testing");
DEFINE_string(log_dir_info, "/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/info", "config log dir for testing");
DEFINE_string(log_dir_warn, "/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/warn", "config log dir for testing");
DEFINE_string(log_dir_error, "/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/error", "config log dir for testing");
DEFINE_string(log_dir_fatal, "/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/fatal", "config log dir for testing");
