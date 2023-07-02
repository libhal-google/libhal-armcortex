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

#include <libhal-armcortex/dwt_counter.hpp>

#include "dwt_counter_reg.hpp"

namespace hal::cortex_m {
dwt_counter::dwt_counter(hertz p_cpu_frequency)
  : m_cpu_frequency(p_cpu_frequency)
{
  // Enable trace core
  core->demcr = (core->demcr | core_trace_enable);

  // Reset cycle count
  dwt->cyccnt = 0;

  // Start cycle count
  dwt->ctrl = (dwt->ctrl | enable_cycle_count);
}

void dwt_counter::register_cpu_frequency(hertz p_cpu_frequency)
{
  m_cpu_frequency = p_cpu_frequency;
}

dwt_counter::uptime_t dwt_counter::driver_uptime()
{
  return uptime_t{ .ticks = m_uptime.update(dwt->cyccnt) };
}

dwt_counter::frequency_t dwt_counter::driver_frequency()
{
  return frequency_t{ .operating_frequency = m_cpu_frequency };
}
}  // namespace hal::cortex_m
