set(TEST_EXECUTABLE_NAME "${PROJECT_NAME}.test.exe")

find_package(ut)

add_executable(${TEST_EXECUTABLE_NAME}
${PROJECT_NAME}/system_timer.test.cpp
${PROJECT_NAME}/dwt_counter.test.cpp
${PROJECT_NAME}/interrupt.test.cpp
tests/main.test.cpp)

target_compile_options(${TEST_EXECUTABLE_NAME} PRIVATE -g -O0)
target_link_libraries(${TEST_EXECUTABLE_NAME}
                      PRIVATE
                      ${PROJECT_NAME}
                      libembeddedhal::libembeddedhal
                      ut)
