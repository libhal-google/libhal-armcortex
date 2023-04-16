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

#include <boost/ut.hpp>

namespace hal::cortex_m {
void dwt_test()
{
  using namespace boost::ut;
  using namespace hal::cortex_m;

  static constexpr auto operating_frequency = 1'000.0_MHz;

  "dwt_counter::ctor()"_test = []() {
    dwt_counter test_subject(operating_frequency);
    expect(that % cortex_m::dwt_counter::core_trace_enable ==
           cortex_m::dwt_counter::core()->demcr);
  };

  "dwt_counter::count()"_test = []() {
    dwt_counter test_subject(operating_frequency);
    {
      cortex_m::dwt_counter::dwt()->cyccnt = 0;
      auto count = test_subject.uptime().value().ticks;
      expect(that % 0 == count);
    }
    {
      cortex_m::dwt_counter::dwt()->cyccnt = 17;
      auto count = test_subject.uptime().value().ticks;
      expect(that % 17 == count);
    }
    {
      cortex_m::dwt_counter::dwt()->cyccnt = 1024;
      auto count = test_subject.uptime().value().ticks;
      expect(that % 1024 == count);
    }
    {
      cortex_m::dwt_counter::dwt()->cyccnt = 5;
      auto count = test_subject.uptime().value().ticks;
      expect(that % (1ULL << 32 | 5) == count);
    }
    {
      cortex_m::dwt_counter::dwt()->cyccnt = 4;
      auto count = test_subject.uptime().value().ticks;
      expect(that % (2ULL << 32 | 4) == count);
    }
    {
      cortex_m::dwt_counter::dwt()->cyccnt = 3;
      auto count = test_subject.uptime().value().ticks;
      expect(that % (3ULL << 32 | 3) == count);
    }
    {
      cortex_m::dwt_counter::dwt()->cyccnt = 10;
      auto count = test_subject.uptime().value().ticks;
      expect(that % (3ULL << 32 | 10) == count);
    }
  };

  "dwt_counter::register_cpu_frequency()"_test = []() {
    dwt_counter test_subject(operating_frequency);
    {
      constexpr auto expected_frequency = 12.0_kHz;
      cortex_m::dwt_counter::dwt()->cyccnt = 0;
      test_subject.register_cpu_frequency(expected_frequency);
      auto count = test_subject.uptime().value().ticks;
      auto freq = test_subject.frequency().value().operating_frequency;
      expect(that % 0 == count);
      expect(that % 0.01f > std::abs(freq - expected_frequency));
    }
    {
      constexpr auto expected_frequency = 99.0_kHz;
      cortex_m::dwt_counter::dwt()->cyccnt = 1337;
      test_subject.register_cpu_frequency(expected_frequency);
      auto count = test_subject.uptime().value().ticks;
      auto freq = test_subject.frequency().value().operating_frequency;
      expect(that % 1337 == count);
      expect(that % 0.01f > std::abs(freq - expected_frequency));
    }
    {
      constexpr auto expected_frequency = 154.0_kHz;
      cortex_m::dwt_counter::dwt()->cyccnt = 65'000'192;
      test_subject.register_cpu_frequency(expected_frequency);
      auto count = test_subject.uptime().value().ticks;
      auto freq = test_subject.frequency().value().operating_frequency;
      expect(that % 65'000'192 == count);
      expect(that % 0.01f > std::abs(freq - expected_frequency));
    }
  };
};
}  // namespace hal::cortex_m
