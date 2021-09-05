#include "system_timer.hpp"

#include <boost/ut.hpp>

using namespace boost::ut;
using namespace cortex_m;

suite system_timer_test = []() {
  system_timer test_subject;
  test_subject.setup_for_unittesting();
  interrupt::setup_for_unittesting();
  interrupt::initialize<20>();

  should("system_timer::attach_interrupt()") = [&] {
    // Setup
    auto dummy_handler = []() {};

    // Exercise
    test_subject.attach_interrupt(dummy_handler);

    // Verify
    expect(interrupt::verify_vector_enabled(test_subject.system_tick_irq,
                                            dummy_handler));
  };

  should("set system_timer::reload_value()") = [&] {
    test_subject.reload_value(5);
    expect(5_i == test_subject.system_tick->LOAD);

    test_subject.reload_value(1024);
    expect(1024_i == test_subject.system_tick->LOAD);

    test_subject.reload_value(1111111);
    expect(1111111_i == test_subject.system_tick->LOAD);

    test_subject.reload_value(1 << 15);
    expect(that % (1 << 15) == test_subject.system_tick->LOAD);
  };

  should("get system_timer::reload_value()") = [&] {
    test_subject.system_tick->LOAD = 5;
    expect(5_i == test_subject.reload_value());

    test_subject.system_tick->LOAD = 1024;
    expect(1024_i == test_subject.reload_value());

    test_subject.system_tick->LOAD = 1111111;
    expect(1111111_i == test_subject.reload_value());

    test_subject.system_tick->LOAD = 1 << 15;
    expect(that % (1 << 15) == test_subject.reload_value());
  };

  should("system_timer::start()") = [&] {
    // Setupfalse
    static constexpr uint32_t ctrl_mask =
      (1 << system_timer::control_bitmap::clk_source) |
      (1 << system_timer::control_bitmap::tick_interrupt) |
      (1 << system_timer::control_bitmap::enable_counter);

    // Exercise
    test_subject.start();

    // Verify
    expect(that % 0 == test_subject.system_tick->VAL);
    expect(that % ctrl_mask == test_subject.system_tick->CTRL);
  };

  should("system_timer::disable()") = [&] {
    // Setup
    test_subject.system_tick->CTRL = 0xA5A5A5A5;

    // Exercise
    test_subject.disable();

    // Verify
    expect(that % 0 == test_subject.system_tick->CTRL);
  };
};
