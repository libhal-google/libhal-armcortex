from conan import ConanFile
from conan.tools.files import copy
from conan.tools.layout import basic_layout
from conan.tools.build import check_min_cppstd
from conan.errors import ConanInvalidConfiguration
import os


required_conan_version = ">=1.50.0"


class LibhalArmCortexConan(ConanFile):
    name = "libhal-armcortex"
    version = "0.3.6"
    license = "Apache-2.0"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://libhal.github.io/libhal-armcortex"
    description = ("A collection of drivers and libraries for the Cortex M "
                   "series ARM processors using libhal")
    topics = ("ARM", "cortex", "cortex-m", "cortex-m0", "cortex-m0+",
              "cortex-m1", "cortex-m3", "cortex-m4", "cortex-m4f", "cortex-m7",
              "cortex-m23", "cortex-m55", "cortex-m35p", "cortex-m33")
    settings = "compiler"
    exports_sources = ("include/*", "linkers/*", "LICENSE", "toolchain.cmake")
    no_copy_source = True

    def package_id(self):
        self.info.clear()

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

    def requirements(self):
        self.requires("libhal/0.3.2")
        self.requires("libhal-util/0.3.3")

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

    def layout(self):
        basic_layout(self)

    def package(self):
        copy(self, "LICENSE", dst=os.path.join(
            self.package_folder, "licenses"),  src=self.source_folder)
        copy(self, "*.h", dst=os.path.join(self.package_folder, "include"),
             src=os.path.join(self.source_folder, "include"))
        copy(self, "*.hpp", dst=os.path.join(self.package_folder,
             "include"), src=os.path.join(self.source_folder, "include"))
        copy(self, "*.ld", dst=os.path.join(self.package_folder,
             "linkers"), src=os.path.join(self.source_folder, "linkers"))
        copy(self, "toolchain.cmake", src=self.source_folder,
             dst=self.package_folder)

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.frameworkdirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.resdirs = []
        linker_path = os.path.join(self.package_folder, "linkers")
        self.cpp_info.exelinkflags = ["-L" + linker_path]
        self.cpp_info.set_property("cmake_target_name", "libhal::armcortex")
        # Add toolchain.cmake to user_toolchain configuration info to be used
        # by CMakeToolchain generator
        f = os.path.join(self.package_folder, "toolchain.cmake")
        self.conf_info.append("tools.cmake.cmaketoolchain:user_toolchain", f)
