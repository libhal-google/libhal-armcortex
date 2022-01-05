#include <boost/ut.hpp>
#include <libarmcortex/system_timer.hpp>

namespace embed::cortex_m {
boost::ut::suite system_timer_test = []() {
  using namespace boost::ut;
  using namespace std::chrono_literals;

  system_timer test_subject(1'000'000 /* MHz */);
  static constexpr size_t ivt_count = 20;
  interrupt::initialize<ivt_count>();


  should("system_timer::driver_initialize()") = [&] {
    // Setup
    // Exercise
    // Verify
  };

  should("system_timer::is_running()") = [&] {
    // Setup
    // Exercise
    // Verify
  };

  should("system_timer::clear()") = [&] {
    // Setup
    // Exercise
    // Verify
  };

  should("system_timer::schedule()") = [&] {
    // Setup
    // Exercise
    // Verify
  };

  should("system_timer::disable()") = [&] {
    // Setup
    // Exercise
    // Verify
  };
};
}
