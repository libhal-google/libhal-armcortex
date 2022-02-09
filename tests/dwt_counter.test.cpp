#include <boost/ut.hpp>
#include <libarmcortex/dwt_counter.hpp>

namespace embed::cortex_m {
boost::ut::suite dwt_test = []() {
  using namespace boost::ut;
  using namespace embed::cortex_m;
  using namespace embed::literals;

  dwt_counter test_subject(1'000_MHz);

  "dwt_counter::start()"_test = [&]() {
    expect(that % cortex_m::dwt_counter::core_trace_enable ==
           cortex_m::dwt_counter::core()->demcr);
  };

  "dwt_counter::count()"_test = [&]() {
    cortex_m::dwt_counter::dwt()->cyccnt = 0;
    expect(that % 0 == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 17;
    expect(that % 17 == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 1024;
    expect(that % 1024 == test_subject.uptime().count());

    // Overflow detection
    cortex_m::dwt_counter::dwt()->cyccnt = 10;
    expect(that % ((1LL << 32) | 10) == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 9;
    expect(that % ((2LL << 32) | 9) == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 8;
    expect(that % ((3LL << 32) | 8) == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 7;
    expect(that % ((4LL << 32) | 7) == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 6;
    expect(that % ((5LL << 32) | 6) == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 5;
    expect(that % ((6LL << 32) | 5) == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 4;
    expect(that % ((7LL << 32) | 4) == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 3;
    expect(that % ((8LL << 32) | 3) == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 2;
    expect(that % ((9LL << 32) | 2) == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 1;
    expect(that % ((10LL << 32) | 1) == test_subject.uptime().count());
    cortex_m::dwt_counter::dwt()->cyccnt = 0;
    expect(that % ((11LL << 32) | 0) == test_subject.uptime().count());
  };
};
}
