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

#include <libhal-armcortex/system_control.hpp>

#include "system_controller_reg.hpp"

#include <libhal/error.hpp>

namespace hal::cortex_m {
void initialize_floating_point_unit()
{
  scb->cpacr = scb->cpacr | ((0b11 << 10 * 2) | /* set CP10 Full Access */
                             (0b11 << 11 * 2)); /* set CP11 Full Access */
}

void set_interrupt_vector_table_address(void* p_table_location)
{
  // Relocate the interrupt vector table the vector buffer. By default this
  // will be set to the address of the start of flash memory for the MCU.
  scb->vtor = reinterpret_cast<intptr_t>(p_table_location);
}

void* get_interrupt_vector_table_address()
{
  // Relocate the interrupt vector table the vector buffer. By default this
  // will be set to the address of the start of flash memory for the MCU.
  return reinterpret_cast<void*>(scb->vtor);  // NOLINT
}

void reset()
{
  // Value "0x5FA" must be written to the VECTKEY field [31:16] to confirm
  // that this action is valid, otherwise the processor ignores the write
  // command.
  // Bit 2 is the SYSRESETREQ bit.
  scb->aircr = (0x5FA << 16) | (1 << 2);
  // System reset is asynchronous, so the code needs to wait.
  hal::halt();
}

void wait_for_interrupt()
{
#if defined(__arm__)
  asm volatile("wfi");
#endif
}

void wait_for_event()
{
#if defined(__arm__)
  asm volatile("wfe");
#endif
}
}  // namespace hal::cortex_m
