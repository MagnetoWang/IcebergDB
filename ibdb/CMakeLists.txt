# include_directories(${INCLUDE_DIRECTORIES} ${PROJECT_SOURCE_DIR}/src)

cmake_minimum_required(VERSION 3.10)
project(ibdb)


set(CMAKE_CXX_STANDARD 11)


# -------------------------------------
# include hub
include_directories(third-party/include)
# 添加自定义头文件目录
# 后面以这个为根目录来引用
include_directories(src)

set(BOOST_ROOT third-party/include)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(gflags REQUIRED)
# -------------------------------------


# -------------------------------------
# ibdb lib

add_library(arena src/base/arena.cpp)
# set_target_properties(libarena PROPERTIES OUTPUT_NAME "arena")


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

set(SOURCE_FILES src/test/map_test.cpp)
add_executable(map_test ${SOURCE_FILES})


# target_link_libraries(test-suite ${Boost_LIBRARIES})
add_executable(boost_test src/test/boost_test.cpp)
target_link_libraries(boost_test Boost::system Boost::filesystem)
# target_link_libraries(boost_test ${Boost_LIBRARIES})
# link_directories(${BOOST_DYLIB})

add_executable(gflags_test src/test/gflags_test.cpp src/flags.cc)
target_link_libraries(gflags_test gflags)

# add_executable(folly_test src/test/folly_test.cpp)
# target_link_libraries(folly_test gflags  lzma folly)
# target_link_libraries(folly_test folly)


# add_executable(atomic_pointer)
add_executable(slice_test src/test/base/slice_test.cpp)


add_executable(base_test src/base/base_test.cpp)
target_link_libraries(base_test arena)
message(${PROJECT_SOURCE_DIR})



