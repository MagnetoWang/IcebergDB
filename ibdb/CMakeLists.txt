cmake_minimum_required(VERSION 3.10)
project(ibdb)

set(CMAKE_CXX_STANDARD 11)

#add_library(ibdb library.cpp library.h src/test/test_map.cpp)
#add_library(ibdb src/test/test_map.cpp)
#set(FILES src/test/test_map.cpp)

set(SOURCE_FILES src/test/test_map.cpp)
add_executable(test_map ${SOURCE_FILES})