#include "system_timer.hpp"

#include <boost/ut.hpp>

using namespace boost::ut;
using namespace cortex_m;

suite system_timer_test = []() {
  system_timer test_subject;
  test_subject.setup_for_unittesting();

  should("system_timer::attach_interrupt()") = [&] {
    // TODO: add later
  };

  should("system_timer::reload_value(uint32_t reload_value)") = [&] {
    test_subject.reload_value(5);
    expect(5_i == test_subject.system_tick->LOAD);

    test_subject.reload_value(1024);
    expect(1024_i == test_subject.system_tick->LOAD);

    test_subject.reload_value(1111111);
    expect(1111111_i == test_subject.system_tick->LOAD);

    test_subject.reload_value(1 << 15);
    expect(that % (1 << 15) == test_subject.system_tick->LOAD);
  };

  should("uint32_t system_timer::reload_value()") = [&] {
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

  };

  should("system_timer::disable()") = [&] {

  };
};
