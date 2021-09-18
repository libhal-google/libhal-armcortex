#include "interrupt.hpp"

#include <boost/ut.hpp>

using namespace boost::ut;
using namespace cortex_m;

suite interrupt_test = [] {
  static constexpr size_t expected_interrupt_count = 42;
  interrupt::setup_for_unittesting();

  expect(that % 16 == interrupt::core_interrupts);

  should("interrupt::initialize()") = [&] {
    // Setup
    // Setup: Set the interrupt_vector_table span to nullptr AND size zero
    interrupt::interrupt_vector_table = std::span<interrupt_pointer>{};

    expect(that % nullptr == interrupt::interrupt_vector_table.data());
    expect(that % 0 == interrupt::interrupt_vector_table.size());

    // Exercise
    interrupt::initialize<expected_interrupt_count>();

    intptr_t pointer =
      reinterpret_cast<intptr_t>(interrupt::interrupt_vector_table.data());
    uint32_t pointer_value = static_cast<uint32_t>(pointer);

    // Verify
    expect(that % nullptr != interrupt::interrupt_vector_table.data());
    expect(that % (expected_interrupt_count + interrupt::core_interrupts) ==
           interrupt::interrupt_vector_table.size());
    expect(that % pointer_value == interrupt::scb->VTOR);
  };

  should("interrupt::enable()") = [&] {
    interrupt_pointer dummy_handler = []() {};

    should("interrupt::enable(5)") = [&]() {
      // Setup
      static constexpr int expected_irq = 5;
      unsigned index = expected_irq >> 5;
      unsigned bit_position = expected_irq & 0x1F;

      // Exercise
      bool success = interrupt(expected_irq).enable(dummy_handler);

      // Verify
      expect(that % success);
      expect(dummy_handler ==
             interrupt::interrupt_vector_table[interrupt::core_interrupts +
                                               expected_irq]);

      expect((1 << bit_position) & interrupt::nvic->ISER[index]);
    };

    should("interrupt::enable(17)") = [&]() {
      // Setup
      static constexpr int expected_irq = 17;
      unsigned index = expected_irq >> 5;
      unsigned bit_position = expected_irq & 0x1F;

      // Exercise
      bool success = interrupt(expected_irq).enable(dummy_handler);

      // Verify
      expect(that % success);
      expect(dummy_handler ==
             interrupt::interrupt_vector_table[interrupt::core_interrupts +
                                               expected_irq]);
      expect((1 << bit_position) & interrupt::nvic->ISER[index]);
    };

    should("interrupt::enable(-5)") = [&]() {
      // Setup
      static constexpr int expected_irq = -5;
      const auto old_nvic = *interrupt::nvic;
      const int iser_size = sizeof(old_nvic.ISER) / sizeof(old_nvic.ISER[0]);

      // Exercise
      bool success = interrupt(expected_irq).enable(dummy_handler);

      // Verify
      expect(that % success);
      // Verify: That the dummy handler was added to the IVT (ISER )
      expect(dummy_handler ==
             interrupt::interrupt_vector_table[interrupt::core_interrupts +
                                               expected_irq]);
      // Verify: ISER[] should not have changed when enable() succeeds but the
      // IRQ is less than 0.
      for (int i = 0; i < iser_size; i++) {
        expect(old_nvic.ISER[i] == interrupt::nvic->ISER[i]);
      }
    };

    should("interrupt::enable(-20) fail") = [&]() {
      // Setup
      static constexpr int expected_irq = -20;
      unsigned index = expected_irq >> 5;
      unsigned bit_position = expected_irq & 0x1F;
      const auto old_nvic = *interrupt::nvic;
      const int iser_size = sizeof(old_nvic.ISER) / sizeof(old_nvic.ISER[0]);

      // Setup: Re-initialize interrupts which will set each vector to "nop"
      interrupt::initialize<expected_interrupt_count>();

      // Exercise
      bool success = interrupt(expected_irq).enable(dummy_handler);

      // Verify
      expect(that % !success);

      // Verify: Nothing in the interrupt vector table should have changed
      for (const auto& interrupt_function : interrupt::interrupt_vector_table) {
        expect(interrupt::nop == interrupt_function);
      }

      // Verify: ISER[] should not have changed when enable() fails.
      for (int i = 0; i < iser_size; i++) {
        expect(old_nvic.ISER[i] == interrupt::nvic->ISER[i]);
      }
    };
  };

  should("interrupt(expected_irq).disable()") = [&] {
    should("interrupt(expected_irq).::disable(5)") = [&]() {
      // Setup
      static constexpr int expected_irq = 5;
      unsigned index = expected_irq >> 5;
      unsigned bit_position = expected_irq & 0x1F;

      // Exercise
      bool success = interrupt(expected_irq).disable();

      // Verify
      expect(that % success);
      expect(interrupt::nop ==
             interrupt::interrupt_vector_table[interrupt::core_interrupts +
                                               expected_irq]);

      expect((1 << bit_position) & interrupt::nvic->ICER[index]);
    };

    should("interrupt(expected_irq).::disable(17)") = [&]() {
      // Setup
      static constexpr int expected_irq = 17;
      unsigned index = expected_irq >> 5;
      unsigned bit_position = expected_irq & 0x1F;

      // Exercise
      bool success = interrupt(expected_irq).disable();

      // Verify
      expect(that % success);
      expect(interrupt::nop ==
             interrupt::interrupt_vector_table[interrupt::core_interrupts +
                                               expected_irq]);
      expect((1 << bit_position) & interrupt::nvic->ICER[index]);
    };

    should("interrupt(expected_irq).disable(-5)") = [&]() {
      // Setup
      static constexpr int expected_irq = -5;
      const auto old_nvic = *interrupt::nvic;
      const int ICER_size = sizeof(old_nvic.ICER) / sizeof(old_nvic.ICER[0]);

      // Exercise
      bool success = interrupt(expected_irq).disable();

      // Verify
      expect(that % success);
      // Verify: That the dummy handler was added to the IVT (ICER )
      expect(interrupt::nop ==
             interrupt::interrupt_vector_table[interrupt::core_interrupts +
                                               expected_irq]);
      // Verify: ICER[] should not have changed when disable() succeeds but the
      // IRQ is less than 0.
      for (int i = 0; i < ICER_size; i++) {
        expect(old_nvic.ICER[i] == interrupt::nvic->ICER[i]);
      }
    };

    should("interrupt(expected_irq).disable(-20) fail") = [&]() {
      // Setup
      static constexpr int expected_irq = -20;
      unsigned index = expected_irq >> 5;
      unsigned bit_position = expected_irq & 0x1F;
      const auto old_nvic = *interrupt::nvic;
      const int ICER_size = sizeof(old_nvic.ICER) / sizeof(old_nvic.ICER[0]);

      // Setup: Re-initialize interrupts which will set each vector to "nop"
      interrupt::initialize<expected_interrupt_count>();

      // Exercise
      bool success = interrupt(expected_irq).disable();

      // Verify
      expect(that % !success);

      // Verify: Nothing in the interrupt vector table should have changed
      for (const auto& interrupt_function : interrupt::interrupt_vector_table) {
        expect(interrupt::nop == interrupt_function);
      }

      // Verify: ICER[] should not have changed when disable() fails.
      for (int i = 0; i < ICER_size; i++) {
        expect(old_nvic.ICER[i] == interrupt::nvic->ICER[i]);
      }
    };
  };

  should("interrupt::get_interrupt_vector_table()") = [&] {
    // Setup
    expect(that % nullptr != interrupt::interrupt_vector_table.data());
    expect(that % 0 != interrupt::interrupt_vector_table.size());

    // Exercise & Verify
    expect(interrupt::interrupt_vector_table.data() ==
           interrupt::get_interrupt_vector_table().data());
    expect(that % interrupt::interrupt_vector_table.size() ==
           interrupt::get_interrupt_vector_table().size());
  };
};
