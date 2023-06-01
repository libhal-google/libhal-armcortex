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
#include <libhal-util/steady_clock.hpp>

int value = 0;

hal::status application()
{
  using namespace hal::literals;
  // Assuming 12 MHz for the CPU clock. Change this to the default CPU clock on
  // the target system.
  hal::cortex_m::dwt_counter counter(12.0_MHz);

  while (true) {
    using namespace std::chrono_literals;
    value++;
    HAL_CHECK(hal::delay(counter, 100ms));
  }

  return hal::success();
}
