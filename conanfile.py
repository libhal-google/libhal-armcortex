from conans import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.layout import cmake_layout


class libarmcortex_conan(ConanFile):
    name = "libarmcortex"
    version = "0.0.1"
    license = "Apache License Version 2.0"
    author = "Khalil Estell"
    url = "https://github.com/SJSU-Dev2/libarmcortex"
    description = "A collection of interfaces and abstractions for embedded peripherals and devices using modern C++"
    topics = ("peripherals", "hardware")
    exports_sources = "CMakeLists.txt", "libarmcortex/*"
    no_copy_source = True
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package_multi"

    def package(self):
        self.copy("*.hpp")

    def package_id(self):
        self.info.header_only()

    def requirements(self):
        self.requires("libembeddedhal/0.0.1@")

    def package_info(self):
        self.cpp_info.includedirs = ["."]
