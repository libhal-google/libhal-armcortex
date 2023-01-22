#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <utility>

#include <libhal/config.hpp>
#include <libhal/error.hpp>

#include "system_control.hpp"

namespace hal::cortex_m {
/// Used specifically for defining an interrupt vector table of addresses.
using interrupt_pointer = void (*)();

/**
 * @brief Cortex M series interrupt controller
 *
 */
class interrupt
{
public:
  /// Structure type to access the Nested Vectored Interrupt Controller (NVIC)
  struct nvic_register_t
  {
    /// Offset: 0x000 (R/W)  Interrupt Set Enable Register
    std::array<volatile uint32_t, 8U> iser;
    /// Reserved 0
    std::array<uint32_t, 24U> reserved0;
    /// Offset: 0x080 (R/W)  Interrupt Clear Enable Register
    std::array<volatile uint32_t, 8U> icer;
    /// Reserved 1
    std::array<uint32_t, 24U> reserved1;
    /// Offset: 0x100 (R/W)  Interrupt Set Pending Register
    std::array<volatile uint32_t, 8U> ispr;
    /// Reserved 2
    std::array<uint32_t, 24U> reserved2;
    /// Offset: 0x180 (R/W)  Interrupt Clear Pending Register
    std::array<volatile uint32_t, 8U> icpr;
    /// Reserved 3
    std::array<uint32_t, 24U> reserved3;
    /// Offset: 0x200 (R/W)  Interrupt Active bit Register
    std::array<volatile uint32_t, 8U> iabr;
    /// Reserved 4
    std::array<uint32_t, 56U> reserved4;
    /// Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide)
    std::array<volatile uint8_t, 240U> ip;
    /// Reserved 5
    std::array<uint32_t, 644U> reserved5;
    /// Offset: 0xE00 ( /W)  Software Trigger Interrupt Register
    volatile uint32_t stir;
  };

  /// NVIC address
  static constexpr intptr_t nvic_address = 0xE000'E100UL;

  /// The core interrupts that all cortex m3, m4, m7 processors have
  static constexpr size_t core_interrupts = 16;

  /// @return auto* - Address of the Nested Vector Interrupt Controller register
  static auto* nvic()
  {
    if constexpr (hal::is_a_test()) {
      static nvic_register_t dummy_nvic{};
      return &dummy_nvic;
    }
    return reinterpret_cast<nvic_register_t*>(nvic_address);
  }

  /// Pointer to a statically allocated interrupt vector table
  static inline std::span<interrupt_pointer> vector_table;

  /**
   * @brief represents an interrupt request number along with helper functions
   * for setting up the interrupt controller registers.
   *
   */
  class exception_number
  {
  public:
    /**
     * @brief construct an exception_number from an int
     *
     * @param p_id - interrupt request number
     */
    constexpr exception_number(std::uint16_t p_id)
      : m_id(p_id)
    {
    }

    constexpr exception_number(exception_number& p_id) = default;
    constexpr exception_number& operator=(exception_number& p_id) = default;

    /**
     * @brief Bits 5 and above represent which 32-bit word in the iser and icer
     * arrays IRQs enable bit resides.
     *
     */
    static constexpr uint32_t index_position = 5;

    /**
     * @brief Lower 5 bits indicate which bit within the 32-bit word is the
     * enable bit.
     *
     */
    static constexpr uint32_t enable_mask_code = 0x1F;

    /**
     * @brief Determines if the irq is within the range of ARM
     *
     * @return true - irq is enabled by default
     * @return false - irq must be enabled to work
     */
    [[nodiscard]] constexpr bool default_enabled() const
    {
      return m_id < core_interrupts;
    }

    [[nodiscard]] constexpr std::uint32_t to_irq_number() const
    {
      return static_cast<std::uint32_t>(m_id - core_interrupts);
    }

    /**
     * @brief the enable bit for this interrupt resides within one of the 32-bit
     * registers within the "iser" and "icer" arrays. This function will return
     * the index of which 32-bit register contains the enable bit.
     *
     * @return constexpr std::uint32_t - array index
     */
    [[nodiscard]] constexpr std::uint32_t register_index() const
    {
      return to_irq_number() >> index_position;
    }

    /**
     * @brief return a mask with a 1 bit in the enable position for this
     * exception_number.
     *
     * @return constexpr std::uint32_t - enable mask
     */
    [[nodiscard]] constexpr std::uint32_t enable_mask() const
    {
      return 1U << (to_irq_number() & enable_mask_code);
    }

    /**
     * @brief
     *
     * @return constexpr size_t
     */
    [[nodiscard]] constexpr size_t vector_index() const
    {
      return m_id;
    }

    /**
     * @brief determines if the irq is within bounds of the interrupt vector
     * table.
     *
     * @return true - is a valid interrupt for this system
     * @return false - this interrupt is beyond the range of valid interrupts
     */
    [[nodiscard]] constexpr bool is_valid() const
    {
      return m_id < vector_table.size();
    }
    /**
     * @return constexpr std::uint16_t - the interrupt request number
     */
    [[nodiscard]] constexpr std::uint16_t get_event_number()
    {
      return m_id;
    }

  private:
    std::uint16_t m_id = 0;
  };

  /**
   * @brief Error indicating that the interrupt vector table is not initialized
   *
   * This error usually indicates that there is a bug in a driver or application
   * because it did not initialize the vector table near the start of the
   * application.
   *
   * But this could also be used to signal to run interrupt::initialize()
   *
   */
  struct vector_table_not_initialized
  {};

  /**
   * @brief An error indicating that an invalid IRQ has been passed to the
   * interrupt class which is outside of the bounds of the interrupt vector
   * table
   *
   * This sort of error is not usually recoverable and indicates an error in a
   * driver.
   *
   */
  struct invalid_irq
  {
    /// Beginning IRQ (always -16)
    static constexpr size_t begin = -core_interrupts;

    /**
     * @brief Construct a new invalid irq object
     *
     * @param p_id - the offending IRQ number
     */
    invalid_irq(exception_number p_id)
      : invalid{ p_id.get_event_number() }
      , end{ vector_table.size() }
    {
    }

    /// Offending IRQ number
    size_t invalid{};
    /// The last IRQ in the table
    size_t end{};
  };

  /// Place holder interrupt that performs no work
  static void nop()
  {
  }

  /**
   * @brief Initializes the interrupt vector table.
   *
   * This template function does the following:
   * - Statically allocates a 512-byte aligned an interrupt vector table the
   *   size of VectorCount.
   * - Set the default handlers for all interrupt vectors to the "nop" function
   *   which does nothing
   * - Set vector_table span to the statically allocated vector table.
   * - Finally it relocates the system's interrupt vector table away from the
   *   hard coded vector table in ROM/Flash memory to the statically allocated
   *   table in RAM.
   *
   * Internally, this function checks if it has been called before and will
   * simply return early if so. Making this function safe to call multiple times
   * so long as the VectorCount template parameter is the same with each
   * invocation.
   *
   * Calling this function with differing VectorCount values will result in
   * multiple statically allocated interrupt vector tables, which will simply
   * waste space in RAM. Only the first call is used as the IVT.
   *
   * @tparam VectorCount - the number of interrupts available for this system
   */
  template<size_t VectorCount>
  static void initialize()
  {
    // Statically allocate a buffer of vectors to be used as the new IVT.
    static constexpr size_t total_vector_count = VectorCount + core_interrupts;
    alignas(512) static std::array<interrupt_pointer, total_vector_count>
      vector_buffer{};

    if (system_control().get_interrupt_vector_table_address() == nullptr) {
      // Assign the statically allocated vector within this scope to the global
      // vector_table span so that it can be accessed in other
      // functions. This is valid because the interrupt vector table has static
      // storage duration and will exist throughout the duration of the
      // application.
      vector_table = vector_buffer;

      // Will fill the interrupt handler and vector table with a function that
      // does nothing.
      std::fill(vector_buffer.begin(), vector_buffer.end(), nop);

      // Relocate the interrupt vector table the vector buffer. By default this
      // will be set to the address of the start of flash memory for the MCU.
      system_control().set_interrupt_vector_table_address(vector_buffer.data());
    }
  }

  /**
   * @brief Reinitialize vector table
   *
   * Will reset the entries of the vector table. Careful to not use this after
   * any drivers have already put entries on to the vector table. This will also
   * disable all interrupts currently enabled on the system.
   *
   * @tparam VectorCount - the number of interrupts available for this system
   */
  template<size_t VectorCount>
  static void reinitialize()
  {
    // Set all bits in the interrupt clear register to 1s to disable those
    // interrupt vectors.
    for (auto& clear_interrupt : nvic()->icer) {
      clear_interrupt = 0xFFFF'FFFF;
    }

    if (hal::is_a_test()) {
      for (auto& set_interrupt : nvic()->iser) {
        set_interrupt = 0x0000'0000;
      }
      for (auto& clear_interrupt : nvic()->icer) {
        clear_interrupt = 0x0000'0000;
      }
    }

    system_control().set_interrupt_vector_table_address(nullptr);
    initialize<VectorCount>();
  }

  /**
   * @brief Get a reference to interrupt vector table object
   *
   * @return const auto& - interrupt vector table
   */
  static const auto& get_vector_table()
  {
    return vector_table;
  }

  /**
   * @brief Construct a new interrupt object
   *
   * @param p_id - interrupt to configure
   */
  explicit interrupt(exception_number p_id)
    : m_id(p_id)
  {
  }

  /**
   * @brief enable interrupt and set the service routine handler.
   *
   * @param p_handler - the interrupt service routine handler to be executed
   * when the hardware interrupt is fired.
   * @return true - successfully installed handler and enabled interrupt
   * @return false - irq value is outside of the bounds of the table
   */
  [[nodiscard]] status enable(interrupt_pointer p_handler)
  {
    HAL_CHECK(sanity_check());

    vector_table[m_id.vector_index()] = p_handler;

    if (!m_id.default_enabled()) {
      nvic_enable_irq();
    }
    return hal::success();
  }

  /**
   * @brief disable interrupt and set the service routine handler to "nop".
   *
   * If the IRQ is invalid, then nothing happens.
   */
  void disable()
  {
    if (!sanity_check()) {
      return;
    }

    vector_table[m_id.vector_index()] = nop;

    if (!m_id.default_enabled()) {
      nvic_disable_irq();
    }
  }

  /**
   * @brief determine if a particular handler has been put into the interrupt
   * vector table.
   *
   * Generally used by unit testing code.
   *
   * @param p_handler - the handler to check against
   * @return true - the handler is equal to the handler in the table
   * @return false -  the handler is not at this index in the table
   */
  [[nodiscard]] result<bool> verify_vector_enabled(interrupt_pointer p_handler)
  {
    HAL_CHECK(sanity_check());

    // Check if the handler match
    auto irq_handler = vector_table[m_id.vector_index()];
    bool handlers_are_the_same = (irq_handler == p_handler);

    if (!handlers_are_the_same) {
      return false;
    }

    if (m_id.default_enabled()) {
      return true;
    }

    uint32_t enable_register = nvic()->iser.at(m_id.register_index());
    return (enable_register & m_id.enable_mask()) == 0U;
  }

private:
  status sanity_check()
  {
    if (!vector_table_is_initialized()) {
      return hal::new_error(vector_table_not_initialized{});
    }

    if (!m_id.is_valid()) {
      return hal::new_error(invalid_irq(m_id));
    }

    return hal::success();
  }

  bool vector_table_is_initialized()
  {
    return system_control().get_interrupt_vector_table_address() != 0x0000'0000;
  }

  /**
   * @brief Enables a device-specific interrupt in the NVIC interrupt
   * controller.
   *
   */
  void nvic_enable_irq()
  {
    auto* interrupt_enable = &nvic()->iser.at(m_id.register_index());
    *interrupt_enable = m_id.enable_mask();
  }

  /**
   * @brief Disables a device-specific interrupt in the NVIC interrupt
   * controller.
   *
   */
  void nvic_disable_irq()
  {
    auto* interrupt_clear = &nvic()->icer.at(m_id.register_index());
    *interrupt_clear = m_id.enable_mask();
  }

  exception_number m_id;
};
}  // namespace hal::cortex_m
