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

namespace hal::cortex_m {
extern void dwt_test();
extern void systick_timer_test();
extern void interrupt_test();
}  // namespace hal::cortex_m

int main()
{
  // [Position Dependent Test]:
  // Initializes interrupt vector table and thus must go first
  hal::cortex_m::interrupt_test();
  hal::cortex_m::dwt_test();
  hal::cortex_m::systick_timer_test();
}
