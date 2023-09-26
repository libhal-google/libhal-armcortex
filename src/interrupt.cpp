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

#include <libhal-armcortex/interrupt.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <utility>

#include <libhal-armcortex/system_control.hpp>

#include "interrupt_reg.hpp"

namespace hal::cortex_m {

namespace {
/// Pointer to a statically allocated interrupt vector table
std::span<interrupt_pointer> vector_table{};
}  // namespace

bool vector_table_is_initialized()
{
  return get_interrupt_vector_table_address() == vector_table.data();
}

/// Place holder interrupt that performs no work
void interrupt::nop()
{
  while (true) {
    continue;
  }
}

bool is_valid_irq_request(const interrupt::exception_number& p_id)
{
  if (!vector_table_is_initialized()) {
    return false;
  }

  if (!p_id.is_valid()) {
    return false;
  }

  if (p_id.vector_index() > vector_table.size()) {
    return false;
  }

  return true;
}

void nvic_enable_irq(const interrupt::exception_number& p_id)
{
  auto* interrupt_enable = &nvic->iser[p_id.register_index()];
  *interrupt_enable = p_id.enable_mask();
}

void nvic_disable_irq(const interrupt::exception_number& p_id)
{
  auto* interrupt_clear = &nvic->icer[p_id.register_index()];
  *interrupt_clear = p_id.enable_mask();
}

const std::span<interrupt_pointer> interrupt::get_vector_table()
{
  return vector_table;
}

interrupt::interrupt(exception_number p_id)
  : m_id(p_id)
{
}

void interrupt::enable(interrupt_pointer p_handler)
{
  if (!is_valid_irq_request(m_id)) {
    return;
  }

  vector_table[m_id.vector_index()] = p_handler;

  if (!m_id.default_enabled()) {
    nvic_enable_irq(m_id);
  }
}

void interrupt::disable()
{
  if (!is_valid_irq_request(m_id)) {
    return;
  }

  vector_table[m_id.vector_index()] = nop;

  if (!m_id.default_enabled()) {
    nvic_disable_irq(m_id);
  }
}

bool interrupt::verify_vector_enabled(interrupt_pointer p_handler)
{
  if (!is_valid_irq_request(m_id)) {
    return false;
  }

  // Check if the handler match
  auto irq_handler = vector_table[m_id.vector_index()];
  bool handlers_are_the_same = (irq_handler == p_handler);

  if (!handlers_are_the_same) {
    return false;
  }

  if (m_id.default_enabled()) {
    return true;
  }

  uint32_t enable_register = nvic->iser[m_id.register_index()];
  return (enable_register & m_id.enable_mask()) == 0U;
}

void interrupt::reset()
{
  disable_interrupts();

  // Set all bits in the interrupt clear register to 1s to disable those
  // interrupt vectors.
  for (auto& clear_interrupt : nvic->icer) {
    clear_interrupt = 0xFFFF'FFFF;
  }

  // Reset vector table
  vector_table = std::span<interrupt_pointer>();
}

void interrupt::setup(std::span<interrupt_pointer> p_vector_table)
{
  if (p_vector_table.data() == vector_table.data() &&
      p_vector_table.size() == vector_table.size()) {
    return;
  }

  // Assign the statically allocated vector within this scope to the global
  // vector_table span so that it can be accessed in other
  // functions. This is valid because the interrupt vector table has static
  // storage duration and will exist throughout the duration of the
  // application.
  vector_table = p_vector_table;

  // Copy the "top-of-stack" from the original vector table
  vector_table[0] = reinterpret_cast<interrupt_pointer*>(
    get_interrupt_vector_table_address())[0];

  // Copy the "reset" handler from the original vector table
  vector_table[1] = reinterpret_cast<interrupt_pointer*>(
    get_interrupt_vector_table_address())[1];

  // Fill the interrupt handler and vector table with a function that does
  // nothing functions. Skip the first 2 which are the top of stock and reset
  // handlers.
  std::fill(vector_table.begin() + 2, vector_table.end(), &nop);

  disable_interrupts();

  // Relocate the interrupt vector table the vector buffer. By default this
  // will be set to the address of the start of flash memory for the MCU.
  set_interrupt_vector_table_address(vector_table.data());

  enable_interrupts();
}

void interrupt::disable_interrupts()
{
#if defined(__arm__)
  asm volatile("cpsid i" : : : "memory");
#endif
}

void interrupt::enable_interrupts()
{
#if defined(__arm__)
  asm volatile("cpsie i" : : : "memory");
#endif
}
}  // namespace hal::cortex_m
