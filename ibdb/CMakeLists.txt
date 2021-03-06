# include_directories(${INCLUDE_DIRECTORIES} ${PROJECT_SOURCE_DIR}/src)

cmake_minimum_required(VERSION 3.10)
project(ibdb)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_WARN_DEPRECATED true)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# -------------------------------------
# include hub
include_directories(third-party/include)
include_directories(third-party/include/google)
include_directories(third-party/include/brpc)
# 添加自定义头文件目录
# 后面以这个为根目录来引用
include_directories(src)
# include_directories(/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/Kernel.framework/Versions/A/Headers/machine)

set(BOOST_ROOT third-party)

find_package(Boost 1.68.0 COMPONENTS COMPONENTS system filesystem)
if(Boost_FOUND)

    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    include_directories(${Boost_INCLUDE_DIRS})

endif()

find_package(gflags REQUIRED)
find_package(glog REQUIRED)
# find_package(protobuf REQUIRED)
# if(PROTOBUF_FOUND)
#     message(STATUS "protobuf library found")
# else()
#     message(FATAL_ERROR "protobuf library is needed but cant be found")
# endif()
# -------------------------------------

# -------------------------------------
# lib hub
link_directories(third-party/lib)

# -------------------------------------



# -------------------------------------
# ibdb lib

add_library(base src/base/arena.cpp src/base/crc32c.cpp src/flags.cc)
add_library(log src/log/format.cpp src/log/log_reader.cpp src/log/log_writer.cpp)
add_library(protocc STATIC src/protobuf/storage.pb.cc src/protobuf/rpc.pb.cc)
add_library(storage src/storage/table.cpp)
# set_target_properties(libarena PROPERTIES OUTPUT_NAME "arena")
# add_library(skiplist src/base/skiplist.cpp)

# -------------------------------------


# file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/base/*.cpp)
# set(CMAKE_MACOSX_RPATH 0)

# set(Boost_USE_STATIC_LIBS        OFF)
# set(Boost_USE_MULTITHREADED      ON)
# set(Boost_USE_STATIC_RUNTIME     OFF)
# set(BOOST_ALL_DYN_LINK           ON)   # force dynamic linking for all libraries

# list(APPEND CMAKE_INCLUDE_PATH "/usr/local/include/folly")
# list(APPEND CMAKE_LIBRARY_PATH "/usr/local/lib/folly")
# include_directories(usr/local/include)
# include_directories(third-party/include/include/folly)
# include_directories(third-party/include/gflags)
# include_directories(third-party/include/glog)
# include_directories(third-party/include/double-conversion)
# include_directories(third-party/include/lz4)
# link_libraries(glog build/libglog.dylib)
# link_directories(usr/local/include/folly)
# find_package(folly REQUIRED)
# find_package(gflags REQUIRED)
# find_package(glog REQUIRED)



# set(BOOST_DYLIB third-party/lib)

# Boost_LIBRARIES
# link_directories(${BOOST_DYLIB})
# link_directories(${BOOST_ROOT})
# BOOST_INCLUDEDIR
#add_library(ibdb library.cpp library.h src/test/test_map.cpp)
#add_library(ibdb src/test/test_map.cpp)
#set(FILES src/test/test_map.cpp)

# -------------------------------------
# ibdb test

set(SOURCE_FILES src/test/map_test.cpp)
add_executable(map_test ${SOURCE_FILES})

#add_executable(boost_test src/test/boost_test.cpp)
#target_link_libraries(boost_test Boost::system Boost::filesystem)

add_executable(gflags_test src/test/gflags_test.cpp src/flags.cc)
target_link_libraries(gflags_test gflags)

add_executable(slice_test src/test/base/slice_test.cpp)

add_executable(base_test src/test/base/base_test.cpp)
target_link_libraries(base_test base glog)

add_executable(skiplist_test src/test/base/skiplist_test.cpp)
target_link_libraries(skiplist_test base gtest glog gflags)

add_executable(gtest_test src/test/gtest_test.cpp)
target_link_libraries(gtest_test base gtest)

add_executable(logging_test src/test/base/logging_test.cpp)
target_link_libraries(logging_test base gtest)

add_executable(crc32c_test src/test/base/crc32c_test.cpp)
target_link_libraries(crc32c_test base gtest)

add_executable(log_test src/test/log/log_test.cpp src/flags.cc)
target_link_libraries(log_test base log gtest gflags glog)

add_executable(glog_test src/test/glog_test.cpp src/flags.cc)
target_link_libraries(glog_test glog gtest gflags)

add_executable(thread_test src/test/thread/thread_test.cpp)
target_link_libraries(thread_test gtest)

add_executable(disk_test src/test/storage/disk_test.cpp src/flags.cc)
target_link_libraries(disk_test base log glog gtest gflags)

add_executable(storage_test src/test/protobuf/storage_test.cpp src/flags.cc src/protobuf/storage.pb.cc)
target_link_libraries(storage_test base log protocc protobuf pthread glog gtest gflags)

add_executable(segment_test src/test/storage/segment_test.cpp)
target_link_libraries(segment_test base log protocc protobuf pthread glog gtest gflags)

add_executable(table_test src/test/storage/table_test.cpp)
target_link_libraries(table_test base log protocc protobuf pthread glog gtest gflags)

add_executable(brpc_test src/test/brpc_test.cpp)
target_link_libraries(brpc_test base log protocc protobuf pthread glog gtest gflags brpc)

add_executable(tablet_test src/test/tablet/tablet_test.cpp)
target_link_libraries(tablet_test base log protocc protobuf pthread glog gtest gflags brpc)

add_executable(server_test src/test/client/server_test.cpp)
target_link_libraries(server_test base log protocc protobuf pthread glog gtest gflags zookeeper_mt brpc)

add_executable(client_test src/test/client/client_test.cpp)
target_link_libraries(client_test base log protocc protobuf pthread glog gtest gflags brpc)

add_executable(zk_test src/test/client/zk_test.cpp)
target_link_libraries(zk_test base glog gtest gflags zookeeper_mt)

# add_executable(tablet_node_test src/test/cluster/tablet_node.cpp)
# target_link_libraries(tablet_node_test base log storage protocc protobuf pthread glog gtest gflags zookeeper_mt brpc)

#add_executable(ibdbs src/client/ibdb.cpp)
#target_link_libraries(ibdbs base log protocc protobuf glog gtest gflags zookeeper_mt brpc)

add_executable(performance src/test/performance.cpp)
target_link_libraries(performance base log protocc protobuf glog gtest gflags zookeeper_mt brpc)
# -------------------------------------

message(${PROJECT_SOURCE_DIR})