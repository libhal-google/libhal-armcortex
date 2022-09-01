cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_COMPILER "gcc-11")
set(CMAKE_CXX_COMPILER "g++-11")

project(libhal_tests VERSION 0.0.1 LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_BUILD_TYPE "Debug")

find_package(ut REQUIRED CONFIG)
find_package(libhal REQUIRED CONFIG)
find_package(libxbitset REQUIRED CONFIG)

set(TEST_NAME unit_test)
set(CMAKE_BUILD_TYPE Debug)

add_executable(${TEST_NAME}
  dwt_counter.test.cpp
  interrupt.test.cpp
  main.test.cpp
  systick_timer.test.cpp
)

target_include_directories(${TEST_NAME} PUBLIC . ../include)
target_compile_options(${TEST_NAME} PRIVATE -Werror -Wall -Wextra
  -Wno-unused-function -Wconversion -Wno-psabi)
target_compile_features(${TEST_NAME} PRIVATE cxx_std_20)
set_target_properties(${TEST_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(${TEST_NAME} PRIVATE
  boost::ut
  libhal::libhal
  libxbitset::libxbitset
)
