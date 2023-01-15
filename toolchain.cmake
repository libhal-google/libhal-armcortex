# Append current directory to CMAKE_MODULE_PATH for making device specific
# cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Target definition
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# ------------------------------------------------------------------------------
# Set toolchain paths
# ------------------------------------------------------------------------------
if(NOT DEFINED TOOLCHAIN_PATH)
  set(TOOLCHAIN_PATH "") # assume global location
endif(NOT DEFINED TOOLCHAIN_PATH)

set(TOOLCHAIN arm-none-eabi)

if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif(NOT DEFINED CMAKE_BUILD_TYPE)

# Set system depended extensions
if(WIN32)
  set(TOOLCHAIN_EXT ".exe")
else()
  set(TOOLCHAIN_EXT "")
endif()

# Perform compiler test with static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ------------------------------------------------------------------------------
# Set compiler/linker flags
# ------------------------------------------------------------------------------

# Object build options
#
# -O0                   No optimizations, reduce compilation time and make
# debugging produce the expected results.
# -mthumb               Generate thumb instructions.
# -fno-builtin          Do not use built-in functions provided by GCC.
# -Wall                 Print only standard warnings, for all use Wextra
# -ffunction-sections   Place each function item into its own section in the
# output file.
# -fdata-sections       Place each data item into its own section in the output
# file.
# -fomit-frame-pointer  Omit the frame pointer in functions that don’t need one.
# -mabi=aapcs           Defines enums to be a variable sized type.
set(OBJECT_GEN_FLAGS "-mthumb -fno-builtin -Wall -ffunction-sections \
    -fdata-sections -fomit-frame-pointer -fno-exceptions \
    -fno-rtti -mabi=aapcs")

set(CMAKE_C_FLAGS "${OBJECT_GEN_FLAGS} "
  CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} "
  CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp "
  CACHE INTERNAL "ASM Compiler options")

# -Wl,--gc-sections     Perform the dead code elimination.
# --specs=nano.specs    Link with newlib-nano.
# --specs=nosys.specs   No syscalls, provide empty implementations for the
# POSIX system calls.
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections --specs=nano.specs \
    --specs=nosys.specs -mthumb -mabi=aapcs -Wl,-Map=${CMAKE_PROJECT_NAME}.map"
  CACHE INTERNAL "Linker options")

# ------------------------------------------------------------------------------
# Set debug/release build configuration Options
# ------------------------------------------------------------------------------

# Options for DEBUG build
# -Og   Enables optimizations that do not interfere with debugging.
# -g    Produce debugging information in the operating system’s native format.
set(CMAKE_C_FLAGS_DEBUG "-O0 -g"
  CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g"
  CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "-g"
  CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG ""
  CACHE INTERNAL "Linker options for debug build type")

# Options for RELEASE build
# -Os   Optimize for size. -Os enables all -O2 optimizations.
set(CMAKE_C_FLAGS_RELEASE "-Os -g -DNDEBUG"
  CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g -DNDEBUG"
  CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE ""
  CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE ""
  CACHE INTERNAL "Linker options for release build type")

# ------------------------------------------------------------------------------
# Set compilers
# ------------------------------------------------------------------------------
set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}${TOOLCHAIN}-gcc${TOOLCHAIN_EXT}
  CACHE INTERNAL "C Compiler")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}${TOOLCHAIN}-g++${TOOLCHAIN_EXT}
  CACHE INTERNAL "C++ Compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}${TOOLCHAIN}-gcc${TOOLCHAIN_EXT}
  CACHE INTERNAL "ASM Compiler")
set(CMAKE_SIZE_UTIL ${TOOLCHAIN_PATH}${TOOLCHAIN}-size${TOOLCHAIN_EXT}
  CACHE INTERNAL "size tool")
set(CMAKE_OBJDUMP ${TOOLCHAIN_PATH}${TOOLCHAIN}-objdump${TOOLCHAIN_EXT}
  CACHE INTERNAL "display various information about object files")
set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}${TOOLCHAIN}-objcopy${TOOLCHAIN_EXT}
  CACHE INTERNAL "utility copies the contents of an object file to another")
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}${${TOOLCHAIN}}
  ${CMAKE_PREFIX_PATH} ${CMAKE_BINARY_DIR})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CORTEX_M4F_FLAGS -mcpu=cortex-m4 -mthumb -mtpcs-frame -mtpcs-leaf-frame
  -mfloat-abi=softfp -mfpu=fpv4-sp-d16)

set(CORTEX_M4_FLAGS -mcpu=cortex-m4 -mthumb -mtpcs-frame -mtpcs-leaf-frame
  -mfloat-abi=soft)

function(arm_post_build elf_file)
  find_program(SIZE ${CMAKE_SIZE_UTIL})
  find_program(OBJCOPY ${CMAKE_OBJCOPY})
  find_program(OBJDUMP ${CMAKE_OBJDUMP})

  # Print executable size
  add_custom_command(TARGET ${elf_file} POST_BUILD
    COMMAND ${SIZE} ${CMAKE_CURRENT_BINARY_DIR}/${elf_file})

  # Create hex (intel hex) file
  add_custom_command(TARGET ${elf_file} POST_BUILD
    COMMAND ${OBJCOPY} -O ihex ${CMAKE_CURRENT_BINARY_DIR}/${elf_file}
    ${CMAKE_CURRENT_BINARY_DIR}/${elf_file}.hex)

  # Create bin (binary) file
  add_custom_command(TARGET ${elf_file} POST_BUILD
    COMMAND ${OBJCOPY} -O binary
    ${CMAKE_CURRENT_BINARY_DIR}/${elf_file}
    ${CMAKE_CURRENT_BINARY_DIR}/${elf_file}.bin)

  # Create disassembly file
  add_custom_command(TARGET ${elf_file} POST_BUILD
    COMMAND ${OBJDUMP} --disassemble --demangle
    ${CMAKE_CURRENT_BINARY_DIR}/${elf_file} >
    ${CMAKE_CURRENT_BINARY_DIR}/${elf_file}.S)

  # Create disassembly file with source information
  add_custom_command(TARGET ${elf_file} POST_BUILD
    COMMAND ${OBJDUMP} --all-headers --source
    --disassemble --demangle ${CMAKE_CURRENT_BINARY_DIR}/${elf_file}
    > ${CMAKE_CURRENT_BINARY_DIR}/${elf_file}.lst)
endfunction()
