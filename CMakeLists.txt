cmake_minimum_required(VERSION 3.7)
project(tdap)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig)
pkg_check_modules(CPP_UNIT REQUIRED cppunit>=1.5)

set(HEADER_FILES
        src/tdap/bounds.hpp
        )

set(SOURCE_FILES
        test/main.cpp
        test/TestPower2.hpp
        test/TestPower2.cpp)
include_directories(src/)

add_executable(testsuite ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(testsuite cppunit)

install(FILES ${HEADER_FILES} DESTINATION include/tdap)