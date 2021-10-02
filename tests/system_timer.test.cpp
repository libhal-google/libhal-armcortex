#include <boost/ut.hpp>
#include <libarmcortex/system_timer.hpp>

boost::ut::suite system_timer_test = []() {
  using namespace boost::ut;
  using namespace cortex_m;

  system_timer test_subject;
  static constexpr size_t ivt_count = 20;
  interrupt::initialize<ivt_count>();

  should("system_timer::attach_interrupt()") = [&] {
    // Setup
    auto dummy_handler = []() {};

    // Exercise
    test_subject.attach_interrupt(dummy_handler);

    // Verify
    expect(interrupt(cortex_m::system_timer::irq)
             .verify_vector_enabled(dummy_handler));
  };

  should("set system_timer::reload_value()") = [&] {
    static constexpr std::array dummy_count{ 5U, 1024U, 1111111U, 32768U };

    test_subject.reload_value(dummy_count[0]);
    expect(5_i == cortex_m::system_timer::system_tick->load);

    test_subject.reload_value(dummy_count[1]);
    expect(1024_i == cortex_m::system_timer::system_tick->load);

    test_subject.reload_value(dummy_count[2]);
    expect(1111111_i == cortex_m::system_timer::system_tick->load);

    test_subject.reload_value(dummy_count[3]);
    expect(32768_i == cortex_m::system_timer::system_tick->load);
  };

  should("get system_timer::reload_value()") = [&] {
    static constexpr std::array dummy_count{ 5U, 1024U, 1111111U, 32768U };

    cortex_m::system_timer::system_tick->load = dummy_count[0];
    expect(5_i == test_subject.reload_value());

    cortex_m::system_timer::system_tick->load = dummy_count[1];
    expect(1024_i == test_subject.reload_value());

    cortex_m::system_timer::system_tick->load = dummy_count[2];
    expect(1111111_i == test_subject.reload_value());

    cortex_m::system_timer::system_tick->load = dummy_count[3];
    expect(32768_i == test_subject.reload_value());
  };

  should("system_timer::start()") = [&] {
    // Setup
    static constexpr uint32_t ctrl_mask =
      (1U << system_timer::control_bitmap::clk_source) |
      (1U << system_timer::control_bitmap::tick_interrupt) |
      (1U << system_timer::control_bitmap::enable_counter);

    // Exercise
    test_subject.start();

    // Verify
    expect(that % 0 == cortex_m::system_timer::system_tick->val);
    expect(that % ctrl_mask == cortex_m::system_timer::system_tick->ctrl);
  };

  should("system_timer::disable()") = [&] {
    // Setup
    static constexpr uint32_t dummy_count = 0xA5A5A5A5;
    cortex_m::system_timer::system_tick->ctrl = dummy_count;

    // Exercise
    test_subject.disable();

    // Verify
    expect(that % 0 == cortex_m::system_timer::system_tick->ctrl);
  };
};
