// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <cstdint>
#include <cstring>

// These need to be supplied by the linker script if the application developer
// in order to call hal::cortex::initialize_data_section()
extern "C"
{
  /**
   * @brief this symbol is placed at the start of the data section in RAM.
   *
   */
  extern uint32_t __data_start;
  /**
   * @brief this symbol is place at the start of the data contents in ROM. This
   * is where the globally defined values for each statically allocated variable
   * is saved.
   *
   */
  extern uint32_t __data_source;
  /**
   * @brief This is the length of the data to be copied from ROM to RAM.
   *
   */
  extern uint32_t __data_size;
  /**
   * @brief this symbol is placed at the start of the bss section in RAM.
   *
   */
  extern uint32_t __bss_start;
  /**
   * @brief This is the length of the bss section to write zeros to.
   *
   */
  extern uint32_t __bss_size;
}

namespace hal::cortex_m {
/**
 * @brief Initialize the data section of RAM. This should be the first thing
 * called in main() before using any global or statically allocated variables.
 * It can also be called in the startup code before main is called. This is not
 * done by crt0.s (C runtime startup code) because with an OS, when the
 * executable is copied to RAM, the data section is also copied and those same
 * locations can be reused for the application, removing the need to copy the
 * data section. This will also happen if one loads an elf file to an MCU using
 * a debugger. Typically the RAM section, but not BSS, is copied over. But in
 * the case of the MCU without a debugger, the MCU will have to manage coping
 * the contents from ROM to RAM itself. Systems should always assume they
 * haven't been loaded by any means and should set the data section at the start
 * of the application.
 *
 */
inline void initialize_data_section()
{
  // Initialize statically allocated data by coping the data section from ROM to
  // RAM. CRT0.o/.s does not perform .data section initialization so it must be
  // done by initialize_platform.
  intptr_t data_size = reinterpret_cast<intptr_t>(&__data_size);
  memcpy(&__data_start, &__data_source, data_size);
}
/**
 * @brief Initialize the BSS (uninitialized data section) to all zeros.
 *
 * Not required if the C Runtime 0 (crt0.s/.a/.o) is used as a startup routine.
 */
inline void initialize_bss_section()
{
  // Initialize statically allocated data by coping the data section from ROM to
  // RAM. CRT0.o/.s does not perform .data section initialization so it must be
  // done by initialize_platform.
  intptr_t bss_size = reinterpret_cast<intptr_t>(&__bss_size);
  memset(&__bss_start, 0, bss_size);
}
}  // namespace hal::cortex_m