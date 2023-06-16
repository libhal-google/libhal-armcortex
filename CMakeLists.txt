# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.15)

project(libhal-armcortex VERSION 0.0.1 LANGUAGES CXX)

find_package(libhal REQUIRED CONFIG)
find_package(libhal-util REQUIRED CONFIG)

if(NOT BUILD_TESTING STREQUAL OFF)
  add_subdirectory(tests)
endif()

add_library(libhal-armcortex
  src/system_controller.cpp
  src/dwt_counter.cpp
  src/interrupt.cpp
  src/systick_timer.cpp)

target_include_directories(libhal-armcortex PUBLIC include)
target_compile_features(libhal-armcortex PRIVATE cxx_std_20)
target_compile_options(libhal-armcortex PRIVATE -g)

if(DEFINED LIBHAL_GCC_CPU AND DEFINED LIBHAL_GCC_FLOAT_ABI)
  target_compile_options(libhal-armcortex PRIVATE
    ${LIBHAL_GCC_CPU}
    ${LIBHAL_GCC_FLOAT_ABI}
  )
endif()

target_link_libraries(libhal-armcortex PRIVATE
  libhal::libhal
  libhal::util)

install(TARGETS libhal-armcortex)
