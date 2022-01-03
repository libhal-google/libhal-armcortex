#pragma once

#include <array>
#include <cinttypes>

#include <libembeddedhal/config.hpp>
#include <libembeddedhal/counter/counter.hpp>

namespace embed::cortex_m {
/**
 * @brief A counter with a frequency fixed to the CPU clock rate.
 *
 * This driver is supported for Cortex M3 devices and above.
 * Do not use this counter if the period needs to be changed.
 *
 */
class dwt_counter : public embed::counter
{
public:
  /// Structure type to access the Data Watchpoint and Trace Register (DWT).
  struct dwt_registers_t
  {
    /// Offset: 0x000 (R/W)  Control Register
    volatile uint32_t ctrl;
    /// Offset: 0x004 (R/W)  Cycle Count Register
    volatile uint32_t cyccnt;
    /// Offset: 0x008 (R/W)  CPI Count Register
    volatile uint32_t cpicnt;
    /// Offset: 0x00C (R/W)  Exception Overhead Count Register
    volatile uint32_t exccnt;
    /// Offset: 0x010 (R/W)  Sleep Count Register
    volatile uint32_t sleepcnt;
    /// Offset: 0x014 (R/W)  LSU Count Register
    volatile uint32_t lsucnt;
    /// Offset: 0x018 (R/W)  Folded-instruction Count Register
    volatile uint32_t foldcnt;
    /// Offset: 0x01C (R/ )  Program Counter Sample Register
    volatile const uint32_t pcsr;
    /// Offset: 0x020 (R/W)  Comparator Register 0
    volatile uint32_t comp0;
    /// Offset: 0x024 (R/W)  Mask Register 0
    volatile uint32_t mask0;
    /// Offset: 0x028 (R/W)  Function Register 0
    volatile uint32_t function0;
    /// Reserved 0
    std::array<uint32_t, 1> reserved0;
    /// Offset: 0x030 (R/W)  Comparator Register 1
    volatile uint32_t comp1;
    /// Offset: 0x034 (R/W)  Mask Register 1
    volatile uint32_t mask1;
    /// Offset: 0x038 (R/W)  Function Register 1
    volatile uint32_t function1;
    /// Reserved 1
    std::array<uint32_t, 1> reserved1;
    /// Offset: 0x040 (R/W)  Comparator Register 2
    volatile uint32_t comp2;
    /// Offset: 0x044 (R/W)  Mask Register 2
    volatile uint32_t mask2;
    /// Offset: 0x048 (R/W)  Function Register 2
    volatile uint32_t function2;
    /// Reserved 2
    std::array<uint32_t, 1> reserved2;
    /// Offset: 0x050 (R/W)  Comparator Register 3
    volatile uint32_t comp3;
    /// Offset: 0x054 (R/W)  Mask Register 3
    volatile uint32_t mask3;
    /// Offset: 0x058 (R/W)  Function Register 3
    volatile uint32_t function3;
  };

  /// Structure type to access the Core Debug Register (CoreDebug)
  struct core_debug_registers_t
  {
    /// Offset: 0x000 (R/W)  Debug Halting Control and Status Register
    volatile uint32_t dhcsr;
    /// Offset: 0x004 ( /W)  Debug Core Register Selector Register
    volatile uint32_t dcrsr;
    /// Offset: 0x008 (R/W)  Debug Core Register Data Register
    volatile uint32_t dcrdr;
    /// Offset: 0x00C (R/W)  Debug Exception and Monitor Control Register
    volatile uint32_t demcr;
  };

  /**
   * @brief This bit must be set to 1 to enable use of the trace and debug
   * blocks:
   *
   *   - Data Watchpoint and Trace (DWT)
   *   - Instrumentation Trace Macrocell (ITM)
   *   - Embedded Trace Macrocell (ETM)
   *   - Trace Port Interface Unit (TPIU).
   */
  static constexpr unsigned core_trace_enable = 1 << 24U;

  /// Mask for turning on cycle counter.
  static constexpr unsigned enable_cycle_count = 1 << 0;

  /// Address of the hardware DWT registers
  static constexpr intptr_t dwt_address = 0xE0001000UL;

  /// Address of the Cortex M CoreDebug module
  static constexpr intptr_t core_debug_address = 0xE000EDF0UL;

  /// Pointer to the DWT peripheral
  static inline auto* dwt = reinterpret_cast<dwt_registers_t*>(0xE0001000UL);

  /// Pointer to the Core Debug module
  static inline auto* core =
    reinterpret_cast<core_debug_registers_t*>(core_debug_address);

  /**
   * @brief Setup the application for unit testing which means replacing the
   * register addresses with statically allocated objects.
   *
   */
  static void setup_for_unittesting()
  {
    // Dummy registers for unit testing
    static dwt_registers_t dummy_dwt{};
    static core_debug_registers_t dummy_core{};

    // Replace the address of the peripheral pointer with the dummy structure so
    // that they can be inspected during unit tests.
    dwt = &dummy_dwt;
    core = &dummy_core;
  }

  /**
   * @brief Construct a new dwt counter object
   *
   */
  dwt_counter()
    : m_previous_count(0)
    , m_overflow_count(0)
    , m_period(0)
  {
    if constexpr (embed::config::is_a_test()) {
      setup_for_unittesting();
    }
  }

  /**
   * @brief Enable trace control in CoreDebug system as well as stop & reset the
   * counter.
   *
   * @return boost::leaf::result<void> - Never returns an error.
   */
  boost::leaf::result<void> driver_initialize() override
  {
    core->demcr = (core->demcr | core_trace_enable);

    control(controls::stop);
    control(controls::reset);

    return {};
  }

  /**
   * @return boost::leaf::result<bool> returns true if the counter is running.
   * Never returns an error.
   */
  boost::leaf::result<bool> is_running() override
  {
    return (dwt->ctrl & enable_cycle_count) != 0;
  }

  /**
   * @brief Control the behavior of the counter
   *
   * @param p_control - counter control
   * @return boost::leaf::result<void> - never returns an error.
   */
  boost::leaf::result<void> control(controls p_control) override
  {
    switch (p_control) {
      case controls::start:
        dwt->ctrl = (dwt->ctrl | enable_cycle_count);
        break;
      case controls::stop:
        dwt->ctrl = (dwt->ctrl & ~enable_cycle_count);
        break;
      case controls::reset:
        dwt->cyccnt = 0;
        m_previous_count = 0;
        m_overflow_count = 0;
        break;
    }
    return {};
  }

  /**
   * @brief The period setter does not work like a typical counter. Typical
   * counters must use the period given here to generate a count that fits that
   * period. But the period for a DWT counter is determined by the cycle count
   * of the ARM Cortex Mx CPU. The period should be set with that value so that
   * it can be returned the period() function. If a set period must be used for
   * your application, then the dwt counter should not be used.
   *
   * @param p_period - operating period of the DWT counter NOT the period to set
   * the DWT counter to
   * @return boost::leaf::result<void> - always successful value
   */
  boost::leaf::result<void> period(std::chrono::nanoseconds p_period) override
  {
    m_period = p_period;
    return {};
  }

  /**
   * @brief Return previously set period
   *
   * @return boost::leaf::result<std::chrono::nanoseconds> previously set
   * period, will not return an error.
   */
  boost::leaf::result<std::chrono::nanoseconds> period() override
  {
    return m_period;
  }

  /**
   * @brief Return the current number of CPU Cycles of the CPU
   *
   * @return boost::leaf::result<uint64_t>
   */
  boost::leaf::result<uint64_t> count() override { return count64(); }

private:
  /// Return the current number of cycles of the CPU
  uint32_t count32()
  {
    m_previous_count = dwt->cyccnt;
    return m_previous_count;
  }

  /// Return the current number of ticks CPU and detects overflows which can be
  /// used to get uptime durations up to 2^64.
  uint64_t count64()
  {
    auto current_count = count32();

    if (m_previous_count > current_count) {
      m_overflow_count++;
    }

    m_previous_count = current_count;

    uint64_t combined_count = m_overflow_count;
    combined_count <<= 32;
    combined_count |= current_count;

    return combined_count;
  }

  uint32_t m_previous_count = 0;
  uint32_t m_overflow_count = 0;
  std::chrono::nanoseconds m_period;
};
}  // namespace embed::cortex_m
