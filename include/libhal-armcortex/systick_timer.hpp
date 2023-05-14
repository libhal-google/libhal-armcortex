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

#include <libhal-util/units.hpp>
#include <libhal/timer.hpp>

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
  /**
   * @brief Defines the set of clock sources for the SysTick timer
   *
   */
  enum class clock_source : std::uint8_t
  {
    /// Use an external clock source. What this source is depends on the
    /// architecture and configuration of the platform.
    external = 0,
    /// Use the clock given to the CPU
    processor = 1,
  };

  /**
   * @brief Construct a new systick_timer timer object
   *
   * PRECONDITION: Interrupt vector table must be initialized before creating an
   * instance of this object.
   *
   * @param p_frequency - the clock source's frequency
   * @param p_source - the source of the clock to the systick timer
   */
  systick_timer(hertz p_frequency,
                clock_source p_source = clock_source::processor);

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
                              clock_source p_source = clock_source::processor);

  /**
   * @brief Destroy the system timer object
   *
   * Stop the timer and disable the interrupt service routine.
   */
  ~systick_timer();

private:
  result<is_running_t> driver_is_running() override;
  result<cancel_t> driver_cancel() override;
  result<schedule_t> driver_schedule(hal::callback<void(void)> p_callback,
                                     hal::time_duration p_delay) override;

  hertz m_frequency = 1'000'000.0f;
};
}  // namespace hal::cortex_m
