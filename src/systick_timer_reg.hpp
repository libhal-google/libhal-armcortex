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

#include <libhal-util/bit.hpp>

namespace hal::cortex_m {
/// @brief Structure type to access the System Timer (SysTick).
struct systick_register_t
{
  /// Offset: 0x000 (R/W)  SysTick Control and Status Register
  volatile uint32_t control;
  /// Offset: 0x004 (R/W)  SysTick Reload Value Register
  volatile uint32_t reload;
  /// Offset: 0x008 (R/W)  SysTick Current Value Register
  /// NOTE: Setting this value to anything will zero it out. Setting this zero
  /// will NOT cause the SysTick interrupt to be fired.
  volatile uint32_t current_value;
  /// Offset: 0x00C (R/ )  SysTick Calibration Register
  const volatile uint32_t calib;
};

/// Namespace containing the bitmask objects that are used to manipulate the
/// ARM Cortex Mx SysTick Timer.
namespace systick_control_register {
/// When set to 1, takes the contents of the reload counter, writes it to
/// the current_value register and begins counting down to zero. Setting
/// this to zero stops the counter. Restarting the counter will restart the
/// count.
static constexpr auto enable_counter = hal::bit::mask::from<0>();

/// When SysTick timer's count goes from 1 to 0, if this bit is set, the
/// SysTick interrupt will fire.
static constexpr auto enable_interrupt = hal::bit::mask::from<1>();

/// If set to 0, clock source is external, if set to 1, clock source follows
/// the processor clock.
static constexpr auto clock_source = hal::bit::mask::from<2>();

/// Set to 1 when count falls from 1 to 0. This bit is cleared on the next
/// read of this register.
static constexpr auto count_flag = hal::bit::mask::from<16>();
};  // namespace systick_control_register

/// The address of the sys_tick register
inline constexpr std::intptr_t systick_address = 0xE000'E010UL;
/// The IRQ number for the SysTick interrupt vector
inline constexpr std::uint16_t event_number = 15;

/// @return auto* - Address of the ARM Cortex SysTick peripheral
inline auto* sys_tick = reinterpret_cast<systick_register_t*>(systick_address);
}  // namespace hal::cortex_m
