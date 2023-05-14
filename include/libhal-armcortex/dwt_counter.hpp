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

#include <libhal-util/overflow_counter.hpp>
#include <libhal/steady_clock.hpp>

namespace hal::cortex_m {
/**
 * @brief A counter with a frequency fixed to the CPU clock rate.
 *
 * This driver is supported for Cortex M3 devices and above.
 *
 */
class dwt_counter : public hal::steady_clock
{
public:
  /**
   * @brief Construct a new dwt counter object
   *
   * @param p_cpu_frequency - the operating frequency of the CPU
   */
  dwt_counter(hertz p_cpu_frequency);

  /**
   * @brief Inform the driver of the operating frequency of the CPU in order to
   * generate the correct uptime.
   *
   * Use this when the CPU's operating frequency has changed and no longer
   * matches the frequency supplied to the constructor. Care should be taken
   * when expecting this function when there is the potentially other parts of
   * the system that depend on this counter's uptime to operate.
   *
   * @param p_cpu_frequency - the operating frequency of the CPU
   */
  void register_cpu_frequency(hertz p_cpu_frequency);

private:
  result<uptime_t> driver_uptime() override;
  result<frequency_t> driver_frequency() override;

  overflow_counter<32> m_uptime{};
  hertz m_cpu_frequency{ 1'000'000 };
};
}  // namespace hal::cortex_m
