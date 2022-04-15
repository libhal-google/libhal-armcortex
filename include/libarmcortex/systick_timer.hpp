#pragma once

#include <cstdint>
#include <functional>

#include "interrupt.hpp"

#include <libembeddedhal/config.hpp>
#include <libembeddedhal/frequency.hpp>
#include <libembeddedhal/static_callable.hpp>
#include <libembeddedhal/timer/timer.hpp>
#include <libxbitset/bitset.hpp>

namespace embed::cortex_m {
/**
 * @brief SysTick driver for the ARM Cortex Mx series chips.
 *
 * Available in all ARM Cortex M series processors. Provides a generic and
 * simple timer for every platform using these processor.
 *
 */
class systick_timer : public embed::timer
{
public:
  /// @brief  Structure type to access the System Timer (SysTick).
  struct registers
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
  struct control_register
  {
    /// When set to 1, takes the contents of the reload counter, writes it to
    /// the current_value register and begins counting down to zero. Setting
    /// this to zero stops the counter. Restarting the counter will restart the
    /// count.
    static constexpr auto enable_counter = xstd::bitrange::from<0>();

    /// When SysTick timer's count goes from 1 to 0, if this bit is set, the
    /// SysTick interrupt will fire.
    static constexpr auto enable_interrupt = xstd::bitrange::from<1>();

    /// If set to 0, clock source is external, if set to 1, clock source follows
    /// the processor clock.
    static constexpr auto clock_source = xstd::bitrange::from<2>();

    /// Set to 1 when count falls from 1 to 0. This bit is cleared on the next
    /// read of this register.
    static constexpr auto count_flag = xstd::bitrange::from<16>();
  };

  /**
   * @brief Defines the set of clock sources for the SysTick timer
   *
   */
  enum class clock_source
  {
    /// Use an external clock source. What this source is depends on the
    /// architecture and configuration of the platform.
    external = 0,
    /// Use the clock given to the CPU
    processor = 1,
  };

  /// The address of the sys_tick register
  static constexpr intptr_t address = 0xE000'E010UL;
  /// The IRQ number for the SysTick interrupt vector
  static constexpr int irq = -1;

  /// @return auto* - Address of the ARM Cortex SysTick peripheral
  static auto* sys_tick()
  {
    if constexpr (embed::is_a_test()) {
      static registers dummy_sys_tick{};
      return &dummy_sys_tick;
    }
    return reinterpret_cast<registers*>(address);
  }

  /**
   * @brief Construct a new systick_timer timer object
   *
   * @param p_frequency - the clock source's frequency
   * @param p_source - the source of the clock to the systick timer
   */
  systick_timer(frequency p_frequency,
                clock_source p_source = clock_source::processor)
    : m_frequency(p_frequency)
  {
    register_cpu_frequency(p_frequency, p_source);
  }

  /**
   * @brief Inform the driver of the operating frequency of the CPU in order to
   * generate the correct uptime.
   *
   * Use this when the CPU's operating frequency has changed and no longer
   * matches the frequency supplied to the constructor. Care should be taken
   * when execting this function when there is the potentially other parts of
   * the system that depend on this counter's uptime to operate.
   *
   * This will clear any ongoing scheduled events as the timing will no longer
   * be valid.
   *
   * @param p_frequency - the clock source's frequency
   * @param p_source - the source of the clock to the systick timer
   */
  void register_cpu_frequency(frequency p_frequency,
                              clock_source p_source = clock_source::processor)
  {
    stop();
    m_frequency = p_frequency;

    // Since reloads only occur when the current_value falls from 1 to 0,
    // setting this register directly to zero from any other number will disable
    // reloading of the register and will stop the timer.
    sys_tick()->current_value = 0;

    auto control = xstd::bitmanip(sys_tick()->control);
    control.set(control_register::enable_interrupt);

    if (p_source == clock_source::processor) {
      control.set(control_register::clock_source);
    } else {
      control.reset(control_register::clock_source);
    }

    // Disable the counter if it was previously enabled.
    control.reset(control_register::enable_counter);

    // control will be committed to "sys_tick()->control" on destruction
  }

  /**
   * @brief Destroy the system timer object
   *
   * Stop the timer and disable the interrupt service routine.
   */
  ~systick_timer()
  {
    stop();
    if (!cortex_m::interrupt(irq).disable()) {
      std::abort();
    }
  }

private:
  void start()
  {
    xstd::bitmanip(sys_tick()->control).set(control_register::enable_counter);
  }

  void stop()
  {
    xstd::bitmanip(sys_tick()->control).reset(control_register::enable_counter);
  }

  boost::leaf::result<bool> driver_is_running() noexcept override
  {
    return xstd::bitmanip(sys_tick()->control)
      .test(control_register::enable_counter);
  }

  boost::leaf::result<void> driver_clear() noexcept override
  {
    // All that is needed is to stop the timer. When the timer is started again
    // via `schedule()`, the timer value will be reloaded/reset.
    stop();
    return {};
  }

  boost::leaf::result<void> driver_schedule(
    std::function<void(void)> p_callback,
    std::chrono::nanoseconds p_delay) noexcept override
  {
    static constexpr std::int64_t minimum = 0x00000001;
    static constexpr std::int64_t maximum = 0x00FFFFFF;

    auto cycle_count = BOOST_LEAF_CHECK(m_frequency.cycles_per(p_delay));

    if (minimum < cycle_count && cycle_count <= maximum) {
      auto min_duration =
        BOOST_LEAF_CHECK(m_frequency.duration_from_cycles(minimum));
      auto max_duration =
        BOOST_LEAF_CHECK(m_frequency.duration_from_cycles(maximum));
      return boost::leaf::new_error(out_of_bounds{
        .invalid = p_delay,
        .minimum = min_duration,
        .maximum = max_duration,
      });
    }

    // Stop the previously scheduled event
    stop();

    // Save the p_callback to the static_callable object's statically allocated
    // callback function. The lifetime of this object exists for the duration of
    // the program, so there will never be a dangling reference.
    auto handler = static_callable<systick_timer, 0, void(void)>(p_callback);

    // Enable interrupt service routine for SysTick and use this callback as the
    // handler
    BOOST_LEAF_CHECK(cortex_m::interrupt(irq).enable(handler.get_handler()));

    // Set the time reload value
    sys_tick()->reload = static_cast<uint32_t>(cycle_count);

    // Starting the timer will restart the count
    start();

    return {};
  }

  frequency m_frequency = frequency(1'000'000);
};
}  // namespace embed::cortex_m
