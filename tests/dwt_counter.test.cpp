#include <boost/ut.hpp>
#include <libarmcortex/dwt_counter.hpp>

using namespace boost::ut;
using namespace cortex_m;

suite dwt_test = []() {
  dwt_counter test_subject;

  "dwt_counter::start()"_test = [&]() {
    test_subject.start();

    expect(that % test_subject.core_trace_enable == test_subject.core->demcr);
    expect(that % 0 == test_subject.dwt->cyccnt);
    expect(that % test_subject.enable_dwt_cycle_count ==
           test_subject.dwt->ctrl);
  };

  "dwt_counter::count()"_test = [&]() {
    test_subject.dwt->cyccnt = 0;
    expect(that % 0 == test_subject.count());
    test_subject.dwt->cyccnt = 17;
    expect(that % 17 == test_subject.count());
    test_subject.dwt->cyccnt = 1024;
    expect(that % 1024 == test_subject.count());
  };
};
