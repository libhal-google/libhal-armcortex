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

/**
 * @brief libhal drivers for the ARM Cortex-M series of processors
 *
 */
namespace hal::cortex_m {
/**
 * @brief Enable the floating point unit coprocessor
 *
 * WARNING: If the coprocessor does not exist, as it is optional, a UsageFault
 * will occur. Floating point units are only found within Cortex M4 and
 * above processors.
 *
 */
void initialize_floating_point_unit();

/**
 * @brief Set the address of the systems interrupt vector table
 *
 * The interrupt vector table (IVT) is held in ROM which means that, either
 * the interrupt service routines (ISR) had to be defined at compile time
 * making them immutable at runtime, or that each ISR calls a mutable function
 * pointer which can be changed at runtime.
 *
 * The problem with the first option is that it makes writing and using
 * libraries difficult. Usually requiring updates to the IVT manually by the
 * application designer based on what libraries and drivers the application is
 * using.
 *
 * The second solution has a problem where the additional another layer of
 * indirection increases interrupt latency. A more critical problem of this
 * approach is that many ISRs take advantage of the state of the system when
 * the ISR runs. For example, context switching in an RTOS needs to be able to
 * see the address of where code was when the interrupt occurred and having an
 * additional point of indirection (i.e. calling a function pointer) will
 * change that location from the task to the ISR that called the context
 * switch function. This will usually result in a fault of some sort.
 *
 * Creating an interrupt vector table in RAM and relocating the ISRs there
 * consumes RAM space, but gives great flexibility over the table at runtime.
 *
 * @param p_table_location - address of the interrupt vector table.
 */
void set_interrupt_vector_table_address(void* p_table_location);

/**
 * @brief Get the address of the systems interrupt vector table.
 *
 * On reset the VTOR register is set to 0x0000'0000 or nullptr.
 *
 * @return void* - address within VTOR the interrupt vector table relocation
 * register.
 */
void* get_interrupt_vector_table_address();

/**
 * @brief Request reset from CPU
 *
 */
void reset();
}  // namespace hal::cortex_m
