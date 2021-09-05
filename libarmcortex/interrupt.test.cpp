#include "interrupt.hpp"

#include <boost/ut.hpp>

using namespace boost::ut;
using namespace cortex_m;

suite interrupt_test = [] {
  interrupt::setup_for_unittesting();

  should("interrupt::initialize()") = [&] {
    // Setup
    static constexpr size_t expected_interrupt_count = 42;
    // Setup: Set the interrupt_vector_table span to nullptr AND size zero
    interrupt::interrupt_vector_table = std::span<interrupt_handler>{};

    expect(that % nullptr == interrupt::interrupt_vector_table.data());
    expect(that % 0 == interrupt::interrupt_vector_table.size());

    // Exercise
    interrupt::initialize<expected_interrupt_count>();

    intptr_t pointer =
      reinterpret_cast<intptr_t>(interrupt::interrupt_vector_table.data());
    uint32_t pointer_value = static_cast<uint32_t>(pointer);

    // Verify
    expect(that % nullptr != interrupt::interrupt_vector_table.data());
    expect(that % expected_interrupt_count ==
           interrupt::interrupt_vector_table.size());
    expect(that % pointer_value == interrupt::scb->VTOR);
  };

  should("interrupt::enable()") = [&] {};
  should("interrupt::disable()") = [&] {};
  should("interrupt::get_interrupt_vector_table()") = [&] {};
};
