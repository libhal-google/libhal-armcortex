#include <boost/ut.hpp>
#include <libarmcortex/interrupt.hpp>

namespace hal::cortex_m {
boost::ut::suite interrupt_test = [] {
  using namespace boost::ut;

  static constexpr size_t expected_interrupt_count = 42;

  expect(that % 16 == interrupt::core_interrupts);

  should("interrupt::initialize()") = [&] {
    // Setup
    expect(that % nullptr == interrupt::vector_table.data());
    expect(that % 0 == interrupt::vector_table.size());

    // Exercise
    interrupt::initialize<expected_interrupt_count>();

    auto pointer = reinterpret_cast<intptr_t>(interrupt::vector_table.data());

    // Verify
    expect(that % nullptr != interrupt::vector_table.data());
    expect(that % (expected_interrupt_count + interrupt::core_interrupts) ==
           interrupt::vector_table.size());
    expect(that % pointer == system_control::scb()->vtor);
  };

  should("interrupt::enable()") = [&] {
    interrupt_pointer dummy_handler = []() {};

    should("interrupt::enable(21)") = [&]() {
      // Setup
      static constexpr std::uint16_t expected_event_number = 21;
      static constexpr std::uint16_t shifted_event_number =
        (expected_event_number - interrupt::core_interrupts);
      unsigned index = shifted_event_number >> 5;
      unsigned bit_position = shifted_event_number & 0x1F;

      // Exercise
      bool success = static_cast<bool>(
        interrupt(expected_event_number).enable(dummy_handler));

      // Verify
      expect(that % success);
      expect(that % dummy_handler ==
             interrupt::vector_table[expected_event_number]);
      std::uint32_t iser =
        (1U << bit_position) & interrupt::nvic()->iser.at(index);
      expect(that % (1 << shifted_event_number) == iser);
    };

    should("interrupt::enable(17)") = [&]() {
      // Setup
      static constexpr std::uint16_t expected_event_number = 17;
      static constexpr std::uint16_t shifted_event_number =
        (expected_event_number - interrupt::core_interrupts);
      unsigned index = shifted_event_number >> 5;
      unsigned bit_position = shifted_event_number & 0x1F;

      // Exercise
      bool success = static_cast<bool>(
        interrupt(expected_event_number).enable(dummy_handler));

      // Verify
      expect(that % success);
      expect(that % dummy_handler ==
             interrupt::vector_table[expected_event_number]);
      std::uint32_t iser =
        (1U << bit_position) & interrupt::nvic()->iser.at(index);
      expect(that % (1 << shifted_event_number) == iser);
    };

    should("interrupt::enable(5)") = [&]() {
      // Setup
      static constexpr std::uint16_t expected_event_number = 5;
      const auto old_nvic = *interrupt::nvic();

      // Exercise
      bool success = static_cast<bool>(
        interrupt(expected_event_number).enable(dummy_handler));

      // Verify
      expect(that % success);
      // Verify: That the dummy handler was added to the IVT (ISER)
      expect(that % dummy_handler ==
             interrupt::vector_table[expected_event_number]);
      // Verify: ISER[] should not have changed when enable() succeeds but the
      // IRQ is less than 0.
      for (size_t i = 0; i < old_nvic.iser.size(); i++) {
        expect(old_nvic.iser.at(i) == interrupt::nvic()->iser.at(i));
      }
    };

    should("interrupt::enable(100) fail") = [&]() {
      // Setup
      // Setup: Re-initialize interrupts which will set each vector to "nop"
      interrupt::reinitialize<expected_interrupt_count>();
      static constexpr std::uint16_t expected_event_number = 100;
      const auto old_nvic = *interrupt::nvic();

      // Exercise
      bool success = static_cast<bool>(
        interrupt(expected_event_number).enable(dummy_handler));

      // Verify
      expect(that % !success);

      // Verify: Nothing in the interrupt vector table should have changed
      for (const auto& interrupt_function : interrupt::vector_table) {
        expect(that % &interrupt::nop == interrupt_function);
      }

      // Verify: ISER[] should not have changed when enable() fails.
      for (size_t i = 0; i < old_nvic.iser.size(); i++) {
        expect(old_nvic.iser.at(i) == interrupt::nvic()->iser.at(i));
      }
    };
  };

  should("interrupt(expected_event_number).disable()") = [&] {
    should("interrupt(expected_event_number).disable(21)") = [&]() {
      // Setup
      static constexpr std::uint16_t expected_event_number = 21;
      static constexpr std::uint16_t shifted_event_number =
        (expected_event_number - interrupt::core_interrupts);
      unsigned index = static_cast<uint32_t>(shifted_event_number) >> 5;
      unsigned bit_position =
        static_cast<uint32_t>(shifted_event_number) & 0x1F;

      // Exercise
      bool success =
        static_cast<bool>(interrupt(expected_event_number).disable());

      // Verify
      expect(that % success);
      expect(that % &interrupt::nop ==
             interrupt::vector_table[expected_event_number]);

      std::uint32_t icer =
        (1U << bit_position) & interrupt::nvic()->icer.at(index);
      expect(that % (1 << shifted_event_number) == icer);
    };

    should("interrupt(expected_event_number).disable(17)") = [&]() {
      // Setup
      static constexpr int expected_event_number = 17;
      static constexpr std::uint16_t shifted_event_number =
        (expected_event_number - interrupt::core_interrupts);
      unsigned index = static_cast<uint32_t>(shifted_event_number) >> 5;
      unsigned bit_position =
        static_cast<uint32_t>(shifted_event_number) & 0x1F;

      // Exercise
      bool success =
        static_cast<bool>(interrupt(expected_event_number).disable());

      // Verify
      expect(that % success);
      expect(that % &interrupt::nop ==
             interrupt::vector_table[expected_event_number]);

      std::uint32_t icer =
        (1U << bit_position) & interrupt::nvic()->icer.at(index);
      expect(that % (1 << shifted_event_number) == icer);
    };

    should("interrupt(expected_event_number).disable(5)") = [&]() {
      // Setup
      static constexpr std::uint16_t expected_event_number = 5;
      const auto old_nvic = *interrupt::nvic();

      // Exercise
      bool success =
        static_cast<bool>(interrupt(expected_event_number).disable());

      // Verify
      expect(that % success);
      // Verify: That the dummy handler was added to the IVT (icer )
      expect(that % &interrupt::nop ==
             interrupt::vector_table[expected_event_number]);
      // Verify: icer[] should not have changed when disable() succeeds but the
      // IRQ is less than 0.
      for (size_t i = 0; i < old_nvic.icer.size(); i++) {
        expect(old_nvic.icer.at(i) == interrupt::nvic()->icer.at(i));
      }
    };

    should("interrupt(expected_event_number).disable(100) fail") = [&]() {
      // Setup
      // Setup: Re-initialize interrupts which will set each vector to "nop"
      interrupt::reinitialize<expected_interrupt_count>();
      static constexpr int expected_event_number = 100;
      const auto old_nvic = *interrupt::nvic();

      // Exercise
      bool success =
        static_cast<bool>(interrupt(expected_event_number).disable());

      // Verify
      expect(that % !success);

      // Verify: Nothing in the interrupt vector table should have changed
      for (const auto& interrupt_function : interrupt::vector_table) {
        expect(that % &interrupt::nop == interrupt_function);
      }

      // Verify: icer[] should not have changed when disable() fails.
      for (size_t i = 0; i < old_nvic.icer.size(); i++) {
        expect(old_nvic.icer.at(i) == interrupt::nvic()->icer.at(i));
      }
    };
  };

  should("interrupt::get_vector_table()") = [&] {
    // Setup
    expect(that % nullptr != interrupt::vector_table.data());
    expect(that % 0 != interrupt::vector_table.size());

    // Exercise & Verify
    expect(interrupt::vector_table.data() ==
           interrupt::get_vector_table().data());
    expect(that % interrupt::vector_table.size() ==
           interrupt::get_vector_table().size());
  };
};
}
