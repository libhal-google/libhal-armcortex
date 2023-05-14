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
#include <cstdint>

namespace hal::cortex_m {
/// Structure type to access the System Control Block (SCB).
struct scb_registers_t
{
  /// Offset: 0x000 (R/ )  CPUID Base Register
  const volatile uint32_t cpuid;
  /// Offset: 0x004 (R/W)  Interrupt Control and State Register
  volatile uint32_t icsr;
  /// Offset: 0x008 (R/W)  Vector Table Offset Register
  volatile intptr_t vtor;
  /// Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register
  volatile uint32_t aircr;
  /// Offset: 0x010 (R/W)  System Control Register
  volatile uint32_t scr;
  /// Offset: 0x014 (R/W)  Configuration Control Register
  volatile uint32_t ccr;
  /// Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 5)
  std::array<volatile uint8_t, 12U> shp;
  /// Offset: 0x024 (R/W)  System Handler Control and State Register
  volatile uint32_t shcsr;
  /// Offset: 0x028 (R/W)  Configurable Fault Status Register
  volatile uint32_t cfsr;
  /// Offset: 0x02C (R/W)  HardFault Status Register
  volatile uint32_t hfsr;
  /// Offset: 0x030 (R/W)  Debug Fault Status Register
  volatile uint32_t dfsr;
  /// Offset: 0x034 (R/W)  MemManage Fault Address Register
  volatile uint32_t mmfar;
  /// Offset: 0x038 (R/W)  BusFault Address Register
  volatile uint32_t bfar;
  /// Offset: 0x03C (R/W)  Auxiliary Fault Status Register
  volatile uint32_t afsr;
  /// Offset: 0x040 (R/ )  Processor Feature Register
  const std::array<volatile uint32_t, 2U> pfr;
  /// Offset: 0x048 (R/ )  Debug Feature Register
  const volatile uint32_t dfr;
  /// Offset: 0x04C (R/ )  Auxiliary Feature Register
  const volatile uint32_t adr;
  /// Offset: 0x050 (R/ )  Memory Model Feature Register
  const std::array<volatile uint32_t, 4U> mmfr;
  /// Offset: 0x060 (R/ )  Instruction Set Attributes Register
  const std::array<volatile uint32_t, 5U> isar;
  /// Reserved 0
  std::array<uint32_t, 5U> reserved0;
  /// Offset: 0x088 (R/W)  Coprocessor Access Control Register
  volatile uint32_t cpacr;
};

/// System control block address
inline constexpr intptr_t scb_address = 0xE000'ED00UL;

/// @return auto* - Address of the Cortex M system control block register
inline auto* scb = reinterpret_cast<scb_registers_t*>(scb_address);
}  // namespace hal::cortex_m
