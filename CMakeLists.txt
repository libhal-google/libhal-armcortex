cmake_minimum_required(VERSION 3.1...3.21)

project(libarmcortex VERSION 0.0.1 LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_FIND_USE_PACKAGE_REGISTRY true)
find_package(libembeddedhal)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(TARGETS ${PROJECT_NAME}
        NAMESPACE ${PROJECT_NAME}::
        FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake")
export(PACKAGE ${PROJECT_NAME})

set(TEST_EXECUTABLE_NAME "${PROJECT_NAME}.test.exe")

add_executable(${TEST_EXECUTABLE_NAME}
${PROJECT_NAME}/system_timer.test.cpp
${PROJECT_NAME}/dwt_counter.test.cpp
${PROJECT_NAME}/interrupt.test.cpp
${PROJECT_NAME}/main.test.cpp)

target_compile_options(${TEST_EXECUTABLE_NAME} PRIVATE -g -O0)

# target_compile_definitions(${TEST_EXECUTABLE_NAME} PUBLIC PLATFORM="test")
target_link_libraries(${TEST_EXECUTABLE_NAME}
                      PRIVATE
                      ${PROJECT_NAME}
                      libembeddedhal::libembeddedhal
                      CONAN_PKG::boost-ext-ut)
