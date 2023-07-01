#!/usr/bin/python
#
# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.errors import ConanInvalidConfiguration


class demos(ConanFile):
    settings = "compiler", "build_type", "os", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualBuildEnv"

    def layout(self):
        platform_directory = "build/" + str(self.settings.arch.processor)
        cmake_layout(self, build_folder=platform_directory)

    def validate(self):
        valid_build = (self.settings.os == "baremetal" and
                       (self.settings.arch == "thumbv7" or
                        self.settings.arch == "thumbv6" or
                        self.settings.arch == "thumbv8"))

        if not valid_build:
            raise ConanInvalidConfiguration(
                f"Only baremetal OS is allowed here!")

    def build_requirements(self):
        self.tool_requires("libhal-cmake-util/[^1.0.0]")

    def requirements(self):
        self.requires("libhal-armcortex/2.0.0")
        self.requires("libhal-util/2.0.0")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
