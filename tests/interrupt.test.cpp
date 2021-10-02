#include <boost/ut.hpp>
#include <libarmcortex/interrupt.hpp>

boost::ut::suite interrupt_test = [] {
  using namespace boost::ut;
  using namespace cortex_m;

  static constexpr size_t expected_interrupt_count = 42;

  expect(that % 16 == interrupt::core_interrupts);

  should("interrupt::initialize()") = [&] {
    // Setup
    // Setup: Set the interrupt_vector_table span to nullptr AND size zero
    interrupt::interrupt_vector_table = std::span<interrupt_pointer>{};

    expect(that % nullptr == interrupt::interrupt_vector_table.data());
    expect(that % 0 == interrupt::interrupt_vector_table.size());

    // Exercise
    interrupt::initialize<expected_interrupt_count>();

    auto pointer =
      reinterpret_cast<intptr_t>(interrupt::interrupt_vector_table.data());
    auto pointer_value = static_cast<uint32_t>(pointer);

    // Verify
    expect(that % nullptr != interrupt::interrupt_vector_table.data());
    expect(that % (expected_interrupt_count + interrupt::core_interrupts) ==
           interrupt::interrupt_vector_table.size());
    expect(that % pointer_value == interrupt::scb->vtor);
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

      expect((1U << bit_position) & interrupt::nvic->iser.at(index));
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
      expect((1U << bit_position) & interrupt::nvic->iser.at(index));
    };

    should("interrupt::enable(-5)") = [&]() {
      // Setup
      static constexpr int expected_irq = -5;
      const auto old_nvic = *interrupt::nvic;

      // Exercise
      bool success = interrupt(expected_irq).enable(dummy_handler);

      // Verify
      expect(that % success);
      // Verify: That the dummy handler was added to the IVT (ISER)
      expect(dummy_handler ==
             interrupt::interrupt_vector_table[interrupt::core_interrupts +
                                               expected_irq]);
      // Verify: ISER[] should not have changed when enable() succeeds but the
      // IRQ is less than 0.
      for (size_t i = 0; i < old_nvic.iser.size(); i++) {
        expect(old_nvic.iser.at(i) == interrupt::nvic->iser.at(i));
      }
    };

    should("interrupt::enable(-20) fail") = [&]() {
      // Setup
      static constexpr int expected_irq = -20;
      const auto old_nvic = *interrupt::nvic;

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
      for (size_t i = 0; i < old_nvic.iser.size(); i++) {
        expect(old_nvic.iser.at(i) == interrupt::nvic->iser.at(i));
      }
    };
  };

  should("interrupt(expected_irq).disable()") = [&] {
    should("interrupt(expected_irq).::disable(5)") = [&]() {
      // Setup
      static constexpr int expected_irq = 5;
      unsigned index = static_cast<uint32_t>(expected_irq) >> 5;
      unsigned bit_position = static_cast<uint32_t>(expected_irq) & 0x1F;

      // Exercise
      bool success = interrupt(expected_irq).disable();

      // Verify
      expect(that % success);
      expect(interrupt::nop ==
             interrupt::interrupt_vector_table[interrupt::core_interrupts +
                                               expected_irq]);

      expect((1U << bit_position) & interrupt::nvic->icer.at(index));
    };

    should("interrupt(expected_irq).::disable(17)") = [&]() {
      // Setup
      static constexpr int expected_irq = 17;
      unsigned index = static_cast<uint32_t>(expected_irq) >> 5;
      unsigned bit_position = static_cast<uint32_t>(expected_irq) & 0x1F;

      // Exercise
      bool success = interrupt(expected_irq).disable();

      // Verify
      expect(that % success);
      expect(interrupt::nop ==
             interrupt::interrupt_vector_table[interrupt::core_interrupts +
                                               expected_irq]);
      expect((1U << bit_position) & interrupt::nvic->icer.at(index));
    };

    should("interrupt(expected_irq).disable(-5)") = [&]() {
      // Setup
      static constexpr int expected_irq = -5;
      const auto old_nvic = *interrupt::nvic;

      // Exercise
      bool success = interrupt(expected_irq).disable();

      // Verify
      expect(that % success);
      // Verify: That the dummy handler was added to the IVT (icer )
      expect(interrupt::nop ==
             interrupt::interrupt_vector_table[interrupt::core_interrupts +
                                               expected_irq]);
      // Verify: icer[] should not have changed when disable() succeeds but the
      // IRQ is less than 0.
      for (size_t i = 0; i < old_nvic.icer.size(); i++) {
        expect(old_nvic.icer.at(i) == interrupt::nvic->icer.at(i));
      }
    };

    should("interrupt(expected_irq).disable(-20) fail") = [&]() {
      // Setup
      static constexpr int expected_irq = -20;
      const auto old_nvic = *interrupt::nvic;

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

      // Verify: icer[] should not have changed when disable() fails.
      for (size_t i = 0; i < old_nvic.icer.size(); i++) {
        expect(old_nvic.icer.at(i) == interrupt::nvic->icer.at(i));
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
