cmake_minimum_required(VERSION 3.14)
project(tdap)

set(CMAKE_CXX_STANDARD 17)

include_directories(src/tdap /usr/loca/include /usr/include)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost)

set(HEADER_DECL_FILES)
set(HEADER_IMPL_FILES)


set(TEST_SOURCE_FILES
        test/test.cpp)

add_executable(tdap_test ${TEST_SOURCE_FILES} ${HEADER_IMPL_FILES} ${HEADER_DECL_FILES})

add_library(tdap INTERFACE)
install(TARGETS tdap)
