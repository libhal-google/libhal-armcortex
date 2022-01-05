#pragma once

#include <cinttypes>

#include "interrupt.hpp"

#include <libembeddedhal/config.hpp>
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
class system_timer : public embed::timer
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
  enum class clock_source : uint8_t
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

  /// Address of the ARM Cortex SysTick peripheral.
  inline static auto* sys_tick = reinterpret_cast<registers*>(address);

  /**
   * @brief Setup the driver for unit testing
   *
   */
  static void setup_for_unittesting()
  {
    // Dummy registers for unit testing.
    static registers dummy_sys_tick{};

    // Replace the address of the peripheral pointer with the dummy structure so
    // that they can be inspected during unit tests.
    sys_tick = &dummy_sys_tick;
  }

  /**
   * @brief Construct a new system timer object
   *
   * @param p_input_frequency - SysTick timer input clock frequency
   * @param p_source - which clock source will feed the SysTick timer
   */
  system_timer(uint32_t p_input_frequency,
               clock_source p_source = clock_source::processor)
    : m_input_frequency(p_input_frequency)
    , m_source(p_source)

  {
    if constexpr (embed::config::is_a_test()) {
      setup_for_unittesting();
    }
  }

  /**
   * @brief Change the input clock's period length.
   *
   * Will not effect a currently scheduled event.
   *
   * @param p_input_frequency - new input clock period
   */
  void input_frequency(uint32_t p_input_frequency)
  {
    m_input_frequency = p_input_frequency;
  }

  /**
   * @brief Get the current clock period length.
   *
   * @return uint32_t - get the currently assigned clock period
   */
  uint32_t input_frequency() { return m_input_frequency; }

  /**
   * @brief initialize the driver
   *
   * @return boost::leaf::result<void> - will never return an error
   */
  boost::leaf::result<void> driver_initialize() override
  {
    // Since reloads only occur when the current_value falls from 1 to 0,
    // setting this register directly to zero from any other number will disable
    // reloading of the register and will stop the timer.
    sys_tick->current_value = 0;

    auto control = xstd::bitmanip(sys_tick->control);
    control.set(control_register::enable_interrupt);

    if (m_source == clock_source::processor) {
      control.set(control_register::clock_source);
    } else {
      control.reset(control_register::clock_source);
    }

    // Disable the counter if it was previously enabled.
    control.reset(control_register::enable_counter);

    return {};
  }

  /**
   * @brief Determines if the timer has something scheduled.
   *
   * @return boost::leaf::result<bool> - true if the timer has something
   * scheduled and is running, false otherwise.
   * @return boost::leaf::result<bool> - will never return an error
   */
  boost::leaf::result<bool> is_running() override
  {
    return xstd::bitmanip(sys_tick->control)
      .test(control_register::enable_counter);
  }

  /**
   * @brief Clear the currently scheduled event and prevent it from being
   * executed.
   *
   * @return boost::leaf::result<void> - will never return an error
   */
  boost::leaf::result<void> clear() override
  {
    // All that is needed is to stop the timer. When the timer is started again
    // via `schedule()`, the timer value will be reloaded/reset.
    stop();
    return {};
  }

  /**
   * @brief Schedule the timer to call the callback when the expiration delay
   * time has elapsed.
   *
   * @param p_callback - callback to be called when the timer expires.
   * @param p_delay - the amount of time before the callback is called.
   * @return boost::leaf::result<void> - can return `delay_too_small` and
   * `delay_too_large`.
   */
  boost::leaf::result<void> schedule(std::function<void(void)> p_callback,
                                     std::chrono::nanoseconds p_delay) override
  {
    static constexpr std::chrono::nanoseconds minimum(0x00000001);
    static constexpr std::chrono::nanoseconds maximum(0x00FFFFFF);

    // Stop the previously scheduled event
    stop();

    // Save the p_callback to the static_callable object's statically allocated
    // callback function. The lifetime of this object exists for the duration of
    // the program, so there will never be a dangling reference.
    auto handler = static_callable<system_timer, 0, void(void)>(p_callback);

    // Enable interrupt service routine for SysTick using the callback
    cortex_m::interrupt(irq).enable(handler.get_handler());

    // Setup reload interval
    if (p_delay < minimum) {
      return boost::leaf::new_error(delay_too_small{
        .invalid = p_delay,
        .minimum = minimum,
      });
    } else if (p_delay > maximum) {
      return boost::leaf::new_error(delay_too_large{
        .invalid = p_delay,
        .maximum = maximum,
      });
    }

    // Set the time reload value
    sys_tick->reload = p_delay.count();

    // Starting the timer will restart the count
    start();

    return {};
  }

  /**
   * @brief Destroy the system timer object
   *
   * Stop the timer and disable the interrupt service routine.
   *
   */
  ~system_timer()
  {
    stop();
    cortex_m::interrupt(irq).disable();
  }

private:
  void start()
  {
    xstd::bitmanip(sys_tick->control).set(control_register::enable_counter);
  }

  void stop()
  {
    xstd::bitmanip(sys_tick->control).reset(control_register::enable_counter);
  }

  uint32_t m_input_frequency;
  clock_source m_source;
};
}  // namespace embed::cortex_m
