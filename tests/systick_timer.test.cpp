#include <boost/ut.hpp>
#include <libarmcortex/systick_timer.hpp>

namespace hal::cortex_m {
boost::ut::suite systick_timer_test = []() {
  using namespace boost::ut;
  using namespace std::chrono_literals;
  using namespace hal::literals;

  systick_timer test_subject(1_MHz);

  should("systick_timer::systick_timer()") = [&] {
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

  should("systick_timer::~systick_timer()") = [&] {
    // Setup
    // Exercise
    // Verify
  };
};
}
