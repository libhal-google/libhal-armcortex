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

cmake_minimum_required(VERSION 3.20)

# Set project name to demos
project(demos VERSION 0.0.1 LANGUAGES CXX)

# Import the libhal-armcortex library/package (and all of its dependencies)
find_package(libhal-util REQUIRED CONFIG)
find_package(libhal-armcortex REQUIRED CONFIG)

# List of demo applications
# To add a new demo to the list, simply add its filename without the .cpp
# extension to this list.
set(DEMOS dwt_counter)

add_library(startup_code main.cpp)
target_compile_options(startup_code PRIVATE -g)
target_link_libraries(startup_code PRIVATE libhal::armcortex libhal::util)

foreach(demo IN LISTS DEMOS)
    set(current_project ${demo}.elf)
    message(STATUS "Generating Demo for \"${current_project}\"")
    add_executable(${current_project} applications/${demo}.cpp)

    target_compile_features(${current_project} PRIVATE cxx_std_20)
    target_compile_options(${current_project} PRIVATE -g)
    target_link_libraries(${current_project} PRIVATE startup_code
        libhal::armcortex
        libhal::util)

    target_link_options(${current_project} PUBLIC
        -T${CMAKE_SOURCE_DIR}/linker.ld)

    arm_cortex_post_build(${current_project})
endforeach()
