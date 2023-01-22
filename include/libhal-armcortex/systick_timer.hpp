#pragma once

#include <cstdint>

#include <libhal-util/bit.hpp>
#include <libhal-util/static_callable.hpp>
#include <libhal-util/units.hpp>
#include <libhal/config.hpp>
#include <libhal/functional.hpp>
#include <libhal/timer.hpp>

#include "interrupt.hpp"

namespace hal::cortex_m {
/**
 * @brief SysTick driver for the ARM Cortex Mx series chips.
 *
 * Available in all ARM Cortex M series processors. Provides a generic and
 * simple timer for every platform using these processor.
 *
 */
class systick_timer : public hal::timer
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
  static constexpr std::intptr_t address = 0xE000'E010UL;
  /// The IRQ number for the SysTick interrupt vector
  static constexpr std::uint16_t event_number = 15;

  /// @return auto* - Address of the ARM Cortex SysTick peripheral
  static auto* sys_tick()
  {
    if constexpr (hal::is_a_test()) {
      static registers dummy_sys_tick{};
      return &dummy_sys_tick;
    }
    return reinterpret_cast<registers*>(address);
  }

  /**
   * @brief Construct a new systick_timer timer object
   *
   * PRECONDITION: Interrupt vector table must be initialized before using this
   * object.
   *
   * @param p_frequency - the clock source's frequency
   * @param p_source - the source of the clock to the systick timer
   */
  systick_timer(hertz p_frequency,
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
   * when expecting this function when there is the potentially other parts of
   * the system that depend on this counter's uptime to operate.
   *
   * This will clear any ongoing scheduled events as the timing will no longer
   * be valid.
   *
   * @param p_frequency - the clock source's frequency
   * @param p_source - the source of the clock to the systick timer
   */
  void register_cpu_frequency(hertz p_frequency,
                              clock_source p_source = clock_source::processor)
  {
    stop();
    m_frequency = p_frequency;

    // Since reloads only occur when the current_value falls from 1 to 0,
    // setting this register directly to zero from any other number will disable
    // reloading of the register and will stop the timer.
    sys_tick()->current_value = 0;

    auto control = hal::bit::modify(sys_tick()->control);
    control.set<control_register::enable_interrupt>();

    if (p_source == clock_source::processor) {
      control.set<control_register::clock_source>();
    } else {
      control.clear<control_register::clock_source>();
    }

    // Disable the counter if it was previously enabled.
    control.clear<control_register::enable_counter>();

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
    cortex_m::interrupt(event_number).disable();
  }

private:
  void start()
  {
    hal::bit::modify(sys_tick()->control)
      .set<control_register::enable_counter>();
  }

  void stop()
  {
    hal::bit::modify(sys_tick()->control)
      .clear<control_register::enable_counter>();
  }

  result<bool> driver_is_running() override
  {
    return hal::bit::extract<control_register::enable_counter>(
      sys_tick()->control);
  }

  status driver_cancel() override
  {
    // All that is needed is to stop the timer. When the timer is started again
    // via `schedule()`, the timer value will be reloaded/reset.
    stop();
    return success();
  }

  status driver_schedule(hal::callback<void(void)> p_callback,
                         hal::time_duration p_delay) override
  {
    static constexpr std::int64_t maximum = 0x00FFFFFF;

    auto cycle_count = cycles_per(m_frequency, p_delay);
    if (cycle_count <= 1) {
      cycle_count = 1;
    } else if (cycle_count > maximum) {
      auto tick_period = wavelength<std::nano>(m_frequency);
      auto max_duration = HAL_CHECK(duration_from_cycles(m_frequency, maximum));
      return hal::new_error(out_of_bounds{
        .tick_period = tick_period,
        .maximum = max_duration,
      });
    }

    // Stop the previously scheduled event
    stop();

    // Save the p_callback to the static_callable object's statically allocated
    // callback function. The lifetime of this object exists for the duration of
    // the program, so this will never become a dangling reference.
    auto handler = static_callable<systick_timer, 0, void(void)>(p_callback);

    // Enable interrupt service routine for SysTick and use this callback as the
    // handler
    HAL_CHECK(cortex_m::interrupt(event_number).enable(handler.get_handler()));

    // Set the time reload value
    sys_tick()->reload = static_cast<uint32_t>(cycle_count);

    // Starting the timer will restart the count
    start();

    return success();
  }

  hertz m_frequency = 1'000'000.0f;
};
}  // namespace hal::cortex_m
