cmake_minimum_required(VERSION 3.1...3.21)

project(libarmcortex VERSION 0.0.1 LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_FIND_USE_PACKAGE_REGISTRY true)
find_package(libembeddedhal)

add_library(libarmcortex INTERFACE)

target_include_directories(libarmcortex INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>)

target_compile_features(libarmcortex INTERFACE cxx_std_20)
set_target_properties(libarmcortex PROPERTIES CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(TARGETS libarmcortex
        NAMESPACE libarmcortex::
        FILE "${CMAKE_CURRENT_BINARY_DIR}/libarmcortex-config.cmake")
export(PACKAGE libarmcortex)

add_executable(libarmcortex.test.exe
               libarmcortex/system_timer.test.cpp
               libarmcortex/dwt_counter.test.cpp
               libarmcortex/interrupt.test.cpp
               libarmcortex/main.test.cpp)

target_link_libraries(libarmcortex.test.exe
                      PRIVATE
                      libarmcortex
                      libembeddedhal::libembeddedhal
                      CONAN_PKG::boost-ext-ut)
