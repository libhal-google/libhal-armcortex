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
from conan.tools.files import copy
from conan.tools.build import check_min_cppstd
from conan.errors import ConanInvalidConfiguration
import os


required_conan_version = ">=1.50.0"


class libhal_arm_cortex_conan(ConanFile):
    name = "libhal-armcortex"
    version = "2.0.0"
    license = "Apache-2.0"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://libhal.github.io/libhal-armcortex"
    description = ("A collection of drivers and libraries for the Cortex M "
                   "series ARM processors using libhal")
    topics = ("arm", "cortex", "cortex-m", "cortex-m0", "cortex-m0plus",
              "cortex-m1", "cortex-m3", "cortex-m4", "cortex-m4f", "cortex-m7",
              "cortex-m23", "cortex-m55", "cortex-m35p", "cortex-m33")
    settings = "compiler", "build_type", "os", "arch"
    exports_sources = ("include/*", "linker_scripts/*", "tests/*", "src/*",
                       "LICENSE", "CMakeLists.txt")
    generators = "CMakeToolchain", "CMakeDeps", "VirtualBuildEnv"
    no_copy_source = True

    @property
    def _min_cppstd(self):
        return "20"

    @property
    def _compilers_minimum_version(self):
        return {
            "gcc": "11",
            "clang": "14",
            "apple-clang": "14.0.0"
        }

    @property
    def _bare_metal(self):
        return self.settings.os == "baremetal"

    @property
    def _valid_arch_for_baremetal(self):
        if str(self.settings.arch) != "thumbv7em":
            return False

        processors = ["cortex-m0", "cortex-m0plus", "cortex-m1",
                      "cortex-m3", "cortex-m4", "cortex-m4f",
                      "cortex-m7", "cortex-m23", "cortex-m55",
                      "cortex-m35p", "cortex-m33"]
        float_abis = ["soft", "hard"]

        is_gcc = self.settings.compiler == "gcc"
        valid_processor = str(
            self.settings.arch.get_safe("processor")) in processors
        valid_float_abi = str(
            self.settings.arch.get_safe("float_abi")) in float_abis

        return (is_gcc and valid_processor and valid_float_abi)

    def validate(self):
        if self.settings.get_safe("compiler.cppstd"):
            check_min_cppstd(self, self._min_cppstd)

        def lazy_lt_semver(v1, v2):
            lv1 = [int(v) for v in v1.split(".")]
            lv2 = [int(v) for v in v2.split(".")]
            min_length = min(len(lv1), len(lv2))
            return lv1[:min_length] < lv2[:min_length]

        compiler = str(self.settings.compiler)
        version = str(self.settings.compiler.version)
        minimum_version = self._compilers_minimum_version.get(compiler, False)

        if minimum_version and lazy_lt_semver(version, minimum_version):
            raise ConanInvalidConfiguration(
                f"{self.name} {self.version} requires C++{self._min_cppstd}, which your compiler ({compiler}-{version}) does not support")

        if self._bare_metal and not self._valid_arch_for_baremetal:
            raise ConanInvalidConfiguration(
                f"Host settings are not valid for os == 'baremetal'!\n"
                f"settings.compiler must be 'gcc' not { self.settings.compiler }, \n"
                f"settings.arch must be 'thumbv7em' not { self.settings.arch }"
                f"\n"
                f"settings.arch.processor must be: { processors } not "
                f"'{ self.settings.arch.get_safe('processor') }'\n"
                f"settings.arch.float_abi must be: { float_abis } not "
                f"'{ self.settings.arch.get_safe('float_abi') }'"
            )

    def build_requirements(self):
        self.tool_requires("cmake-arm-embedded/1.0.0")

    def requirements(self):
        self.requires("libhal/[^2.0.0]")
        self.requires("libhal-util/[^2.0.0]")
        self.test_requires("boost-ext-ut/1.1.9")

    def layout(self):
        cmake_layout(self)

    def build(self):
        run_test = not self.conf.get("tools.build:skip_test", default=False)

        cmake = CMake(self)
        if self.settings.os == "Windows":
            cmake.configure()
        elif self._bare_metal:
            cmake.configure(variables={
                "BUILD_TESTING": "OFF",
                "LIBHAL_GCC_CPU": f"-mcpu={self.settings.arch.processor }",
                "LIBHAL_GCC_FLOAT_ABI": f"-mfloat-abi={ self.settings.arch.float_abi }",
            })

        else:
            cmake.configure(variables={"ENABLE_ASAN": True})

        cmake.build()

        if run_test and not self._bare_metal:
            test_folder = os.path.join("tests")
            self.run(os.path.join(test_folder, "unit_test"))

    def package(self):
        copy(self,
             "LICENSE",
             dst=os.path.join(self.package_folder, "licenses"),
             src=self.source_folder)

        copy(self,
             "*.h",
             dst=os.path.join(self.package_folder, "include"),
             src=os.path.join(self.source_folder, "include"))
        copy(self,
             "*.hpp",
             dst=os.path.join(self.package_folder, "include"),
             src=os.path.join(self.source_folder, "include"))

        copy(self,
             "*.ld",
             dst=os.path.join(self.package_folder, "linker_scripts"),
             src=os.path.join(self.source_folder, "linker_scripts"))

        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.exelinkflags = []
        self.cpp_info.set_property("cmake_target_name", "libhal::armcortex")
        self.cpp_info.libs = ["libhal-armcortex"]

        if (
            self._bare_metal and
            self.settings.compiler == "gcc" and
            self.settings.arch == "thumbv7em"
        ):
            linker_path = os.path.join(self.package_folder, "linker_scripts")
            self.cpp_info.exelinkflags.append("-L" + linker_path)

            gcc_flags = [
                f"-mcpu={ str(self.settings.arch.processor) }",
                f"-mfloat-abi={ str(self.settings.arch.float_abi) }"
            ]
            linker_flags = ["--specs=nano.specs", "--specs=nosys.specs"]

            self.cpp_info.cflags.extend(gcc_flags)
            self.cpp_info.cxxflags.extend(gcc_flags)
            self.cpp_info.exelinkflags.extend(gcc_flags)
            self.cpp_info.exelinkflags.extend(linker_flags)
