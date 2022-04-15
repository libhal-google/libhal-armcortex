#include <boost/ut.hpp>
#include <libarmcortex/dwt_counter.hpp>

namespace embed::cortex_m {
boost::ut::suite dwt_test = []() {
  using namespace boost::ut;
  using namespace embed::cortex_m;
  using namespace embed::literals;

  constexpr auto operating_frequency = 1'000_MHz;
  dwt_counter test_subject(operating_frequency);

  "dwt_counter::ctor()"_test = [&]() {
    expect(that % cortex_m::dwt_counter::core_trace_enable ==
           cortex_m::dwt_counter::core()->demcr);
  };

  "dwt_counter::count()"_test = [&]() {
    {
      cortex_m::dwt_counter::dwt()->cyccnt = 0;
      auto [frequency, count] = test_subject.uptime().value();
      expect(that % 0 == count);
      expect(operating_frequency == frequency);
    }
    {
      cortex_m::dwt_counter::dwt()->cyccnt = 17;
      auto [frequency, count] = test_subject.uptime().value();
      expect(that % 17 == count);
      expect(operating_frequency == frequency);
    }
    {
      cortex_m::dwt_counter::dwt()->cyccnt = 1024;
      auto [frequency, count] = test_subject.uptime().value();
      expect(that % 1024 == count);
      expect(operating_frequency == frequency);
    }
  };

  "dwt_counter::register_cpu_frequency()"_test = [&]() {
    {
      constexpr auto expected_frequency = 12_kHz;
      cortex_m::dwt_counter::dwt()->cyccnt = 0;
      test_subject.register_cpu_frequency(expected_frequency);
      auto [frequency, count] = test_subject.uptime().value();
      expect(that % 0 == count);
      expect(expected_frequency == frequency);
    }
    {
      constexpr auto expected_frequency = 99_kHz;
      cortex_m::dwt_counter::dwt()->cyccnt = 1337;
      test_subject.register_cpu_frequency(expected_frequency);
      auto [frequency, count] = test_subject.uptime().value();
      expect(that % 1337 == count);
      expect(expected_frequency == frequency);
    }
    {
      constexpr auto expected_frequency = 154_kHz;
      cortex_m::dwt_counter::dwt()->cyccnt = 65'000'192;
      test_subject.register_cpu_frequency(expected_frequency);
      auto [frequency, count] = test_subject.uptime().value();
      expect(that % 65'000'192 == count);
      expect(expected_frequency == frequency);
    }
  };
};
}
