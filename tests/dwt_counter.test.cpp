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
#include "helper.hpp"

#include <boost/ut.hpp>

namespace hal::cortex_m {

void dwt_test()
{
  using namespace boost::ut;
  using namespace hal::cortex_m;

  auto stub_out_core = stub_out_registers(&core);
  auto stub_out_dwt = stub_out_registers(&dwt);

  static constexpr auto operating_frequency = 1'000.0_MHz;

  "dwt_counter::ctor()"_test = []() {
    dwt_counter test_subject(operating_frequency);
    expect(that % core_trace_enable == core->demcr);
  };

  "dwt_counter::count()"_test = []() {
    dwt_counter test_subject(operating_frequency);
    {
      dwt->cyccnt = 0;
      auto count = test_subject.uptime().ticks;
      expect(that % 0 == count);
    }
    {
      dwt->cyccnt = 17;
      auto count = test_subject.uptime().ticks;
      expect(that % 17 == count);
    }
    {
      dwt->cyccnt = 1024;
      auto count = test_subject.uptime().ticks;
      expect(that % 1024 == count);
    }
    {
      dwt->cyccnt = 5;
      auto count = test_subject.uptime().ticks;
      expect(that % (1ULL << 32 | 5) == count);
    }
    {
      dwt->cyccnt = 4;
      auto count = test_subject.uptime().ticks;
      expect(that % (2ULL << 32 | 4) == count);
    }
    {
      dwt->cyccnt = 3;
      auto count = test_subject.uptime().ticks;
      expect(that % (3ULL << 32 | 3) == count);
    }
    {
      dwt->cyccnt = 10;
      auto count = test_subject.uptime().ticks;
      expect(that % (3ULL << 32 | 10) == count);
    }
  };

  "dwt_counter::register_cpu_frequency()"_test = []() {
    dwt_counter test_subject(operating_frequency);
    {
      constexpr auto expected_frequency = 12.0_kHz;
      dwt->cyccnt = 0;
      test_subject.register_cpu_frequency(expected_frequency);
      auto count = test_subject.uptime().ticks;
      auto freq = test_subject.frequency().operating_frequency;
      expect(that % 0 == count);
      expect(that % 0.01f > std::abs(freq - expected_frequency));
    }
    {
      constexpr auto expected_frequency = 99.0_kHz;
      dwt->cyccnt = 1337;
      test_subject.register_cpu_frequency(expected_frequency);
      auto count = test_subject.uptime().ticks;
      auto freq = test_subject.frequency().operating_frequency;
      expect(that % 1337 == count);
      expect(that % 0.01f > std::abs(freq - expected_frequency));
    }
    {
      constexpr auto expected_frequency = 154.0_kHz;
      dwt->cyccnt = 65'000'192;
      test_subject.register_cpu_frequency(expected_frequency);
      auto count = test_subject.uptime().ticks;
      auto freq = test_subject.frequency().operating_frequency;
      expect(that % 65'000'192 == count);
      expect(that % 0.01f > std::abs(freq - expected_frequency));
    }
  };
};
}  // namespace hal::cortex_m
