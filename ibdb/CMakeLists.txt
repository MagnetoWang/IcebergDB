# include_directories(${INCLUDE_DIRECTORIES} ${PROJECT_SOURCE_DIR}/src)

cmake_minimum_required(VERSION 3.10)
project(ibdb)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MACOSX_RPATH 0)

set(Boost_USE_STATIC_LIBS        OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
set(BOOST_ALL_DYN_LINK           ON)   # force dynamic linking for all libraries


set(BOOST_ROOT third-party/include)
set(BOOST_DYLIB third-party/lib)
find_package(Boost REQUIRED COMPONENTS system filesystem)
# Boost_LIBRARIES
link_directories(${BOOST_DYLIB})
# link_directories(${BOOST_ROOT})
# BOOST_INCLUDEDIR
#add_library(ibdb library.cpp library.h src/test/test_map.cpp)
#add_library(ibdb src/test/test_map.cpp)
#set(FILES src/test/test_map.cpp)

set(SOURCE_FILES src/test/map_test.cpp)
add_executable(map_test ${SOURCE_FILES})


INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
# target_link_libraries(test-suite ${Boost_LIBRARIES})
add_executable(boost_test src/test/boost_test.cpp)
target_link_libraries(boost_test Boost::system Boost::filesystem)
target_link_libraries(boost_test ${Boost_LIBRARIES})
# link_directories(${BOOST_DYLIB})




