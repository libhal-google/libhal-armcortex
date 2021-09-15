#pragma once

#include <cinttypes>

#include "interrupt.hpp"

namespace cortex_m {

/// @brief  Structure type to access the System Timer (SysTick).
struct system_tick_t
{
  /// Offset: 0x000 (R/W)  SysTick Control and Status Register
  volatile uint32_t CTRL;
  /// Offset: 0x004 (R/W)  SysTick Reload Value Register
  volatile uint32_t LOAD;
  /// Offset: 0x008 (R/W)  SysTick Current Value Register
  volatile uint32_t VAL;
  /// Offset: 0x00C (R/ )  SysTick Calibration Register
  const volatile uint32_t CALIB;
};

class system_timer
{
public:
  /// Enumeration holding the bit positions of used flags.
  /// Source: "UM10562 LPC408x/407x User manual" table 83 page 132
  enum control_bitmap : uint8_t
  {
    enable_counter = 0,
    tick_interrupt = 1,
    clk_source = 2,
    count_flag = 16
  };

  static constexpr intptr_t system_tick_address = 0xE000'E010UL;
  static constexpr int system_tick_irq = -1;

  /// Address of the ARM Cortex SysTick peripheral.
  inline static auto* system_tick =
    reinterpret_cast<system_tick_t*>(system_tick_address);

  static void setup_for_unittesting()
  {
    // Dummy registers for unit testing
    static system_tick_t dummy_system_tick{};

    // Replace the address of the peripheral pointer with the dummy structure so
    // that they can be inspected during unit tests.
    system_tick = &dummy_system_tick;
  }

  void attach_interrupt(interrupt_handler system_tick_handler)
  {
    cortex_m::interrupt(system_tick_irq).enable(system_tick_handler);
  }

  void reload_value(uint32_t reload_value) { system_tick->LOAD = reload_value; }
  uint32_t reload_value() { return system_tick->LOAD; }

  void start()
  {
    // Set all flags required to enable the counter
    uint32_t ctrl_mask = (1 << control_bitmap::clk_source) |
                         (1 << control_bitmap::tick_interrupt) |
                         (1 << control_bitmap::enable_counter);

    // Set the system tick counter to start immediately
    system_tick->VAL = 0;
    system_tick->CTRL = ctrl_mask;
  }

  void disable() { system_tick->CTRL = 0; }
};
}
