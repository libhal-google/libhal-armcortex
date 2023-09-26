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

project(libhal-armcortex LANGUAGES CXX)

libhal_test_and_make_library(
  LIBRARY_NAME libhal-armcortex

  SOURCES
  src/system_controller.cpp
  src/dwt_counter.cpp
  src/interrupt.cpp
  src/systick_timer.cpp

  TEST_SOURCES
  tests/dwt_counter.test.cpp
  tests/interrupt.test.cpp
  tests/main.test.cpp
  tests/systick_timer.test.cpp

  PACKAGES
  libhal
  libhal-util

  LINK_LIBRARIES
  libhal::libhal
  libhal::util
)
