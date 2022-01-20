#include <boost/ut.hpp>
#include <libarmcortex/systick_timer.hpp>

namespace embed::cortex_m {
boost::ut::suite systick_timer_test = []() {
  using namespace boost::ut;
  using namespace std::chrono_literals;
  using namespace embed::literals;

  systick_timer test_subject(1_MHz);
  static constexpr size_t ivt_count = 20;
  interrupt::initialize<ivt_count>();

  should("systick_timer::driver_initialize()") = [&] {
    // Setup
    // Exercise
    // Verify
  };

  should("systick_timer::is_running()") = [&] {
    // Setup
    // Exercise
    // Verify
  };

  should("systick_timer::clear()") = [&] {
    // Setup
    // Exercise
    // Verify
  };

  should("systick_timer::schedule()") = [&] {
    // Setup
    // Exercise
    // Verify
  };

  should("systick_timer::disable()") = [&] {
    // Setup
    // Exercise
    // Verify
  };
};
}
