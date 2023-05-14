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

#include <array>

namespace hal::cortex_m {

/// Structure type to access the Nested Vectored Interrupt Controller (NVIC)
struct nvic_register_t
{
  /// Offset: 0x000 (R/W)  Interrupt Set Enable Register
  std::array<volatile uint32_t, 8U> iser;
  /// Reserved 0
  std::array<uint32_t, 24U> reserved0;
  /// Offset: 0x080 (R/W)  Interrupt Clear Enable Register
  std::array<volatile uint32_t, 8U> icer;
  /// Reserved 1
  std::array<uint32_t, 24U> reserved1;
  /// Offset: 0x100 (R/W)  Interrupt Set Pending Register
  std::array<volatile uint32_t, 8U> ispr;
  /// Reserved 2
  std::array<uint32_t, 24U> reserved2;
  /// Offset: 0x180 (R/W)  Interrupt Clear Pending Register
  std::array<volatile uint32_t, 8U> icpr;
  /// Reserved 3
  std::array<uint32_t, 24U> reserved3;
  /// Offset: 0x200 (R/W)  Interrupt Active bit Register
  std::array<volatile uint32_t, 8U> iabr;
  /// Reserved 4
  std::array<uint32_t, 56U> reserved4;
  /// Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide)
  std::array<volatile uint8_t, 240U> ip;
  /// Reserved 5
  std::array<uint32_t, 644U> reserved5;
  /// Offset: 0xE00 ( /W)  Software Trigger Interrupt Register
  volatile uint32_t stir;
};

/// NVIC address
inline constexpr intptr_t nvic_address = 0xE000'E100UL;

inline auto* nvic = reinterpret_cast<nvic_register_t*>(nvic_address);
}  // namespace hal::cortex_m
