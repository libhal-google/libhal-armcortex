#include <boost/ut.hpp>
#include <libarmcortex/dwt_counter.hpp>

boost::ut::suite dwt_test = []() {
  using namespace boost::ut;
  using namespace cortex_m;

  dwt_counter test_subject;

  "dwt_counter::start()"_test = [&]() {
    test_subject.start();

    expect(that % cortex_m::dwt_counter::core_trace_enable ==
           cortex_m::dwt_counter::core->demcr);
    expect(that % 0 == cortex_m::dwt_counter::dwt->cyccnt);
    expect(that % cortex_m::dwt_counter::enable_dwt_cycle_count ==
           cortex_m::dwt_counter::dwt->ctrl);
  };

  "dwt_counter::count()"_test = [&]() {
    cortex_m::dwt_counter::dwt->cyccnt = 0;
    expect(that % 0 == test_subject.count());
    cortex_m::dwt_counter::dwt->cyccnt = 17;
    expect(that % 17 == test_subject.count());
    cortex_m::dwt_counter::dwt->cyccnt = 1024;
    expect(that % 1024 == test_subject.count());
  };
};
