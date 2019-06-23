cmake_minimum_required(VERSION 3.14)
project(tdap)

set(CMAKE_CXX_STANDARD 17)

include_directories(src /usr/local/include /usr/include)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost)

set(HEADER_FILES
        src/tdap/average.hpp
        src/tdap/filter.hpp
        src/tdap/macros.hpp
        src/tdap/boundaries.hpp)

set(HEADER_IMPL_FILES
        src/tdap/impl/average-impl.hpp
        src/tdap/impl/filter-impl.hpp
        src/tdap/impl/boundaries-helper.hpp
        src/tdap/impl/average-helper.hpp)

set(TEST_SOURCE_FILES
        test/test.cpp)

add_executable(tdap_test ${TEST_SOURCE_FILES} ${HEADER_IMPL_FILES} ${HEADER_FILES})

add_library(tdap INTERFACE)
install(TARGETS tdap)
