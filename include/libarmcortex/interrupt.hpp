#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <utility>

#include <libembeddedhal/config.hpp>

namespace embed::cortex_m {
/// Used specifically for defining an interrupt vector table of addresses.
using interrupt_pointer = void (*)();

/**
 * @brief Cortex M series interrupt controller
 *
 */
class interrupt
{
public:
  /// Structure type to access the System Control Block (SCB).
  struct scb_registers_t
  {
    /// Offset: 0x000 (R/ )  CPUID Base Register
    const volatile uint32_t cpuid;
    /// Offset: 0x004 (R/W)  Interrupt Control and State Register
    volatile uint32_t icsr;
    /// Offset: 0x008 (R/W)  Vector Table Offset Register
    volatile uint32_t vtor;
    /// Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register
    volatile uint32_t aircr;
    /// Offset: 0x010 (R/W)  System Control Register
    volatile uint32_t scr;
    /// Offset: 0x014 (R/W)  Configuration Control Register
    volatile uint32_t ccr;
    /// Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 5)
    std::array<volatile uint8_t, 12U> shp;
    /// Offset: 0x024 (R/W)  System Handler Control and State Register
    volatile uint32_t shcsr;
    /// Offset: 0x028 (R/W)  Configurable Fault Status Register
    volatile uint32_t cfsr;
    /// Offset: 0x02C (R/W)  HardFault Status Register
    volatile uint32_t hfsr;
    /// Offset: 0x030 (R/W)  Debug Fault Status Register
    volatile uint32_t dfsr;
    /// Offset: 0x034 (R/W)  MemManage Fault Address Register
    volatile uint32_t mmfar;
    /// Offset: 0x038 (R/W)  BusFault Address Register
    volatile uint32_t bfar;
    /// Offset: 0x03C (R/W)  Auxiliary Fault Status Register
    volatile uint32_t afsr;
    /// Offset: 0x040 (R/ )  Processor Feature Register
    const std::array<volatile uint32_t, 2U> pfr;
    /// Offset: 0x048 (R/ )  Debug Feature Register
    const volatile uint32_t dfr;
    /// Offset: 0x04C (R/ )  Auxiliary Feature Register
    const volatile uint32_t adr;
    /// Offset: 0x050 (R/ )  Memory Model Feature Register
    const std::array<volatile uint32_t, 4U> mmfr;
    /// Offset: 0x060 (R/ )  Instruction Set Attributes Register
    const std::array<volatile uint32_t, 5U> isar;
    std::array<uint32_t, 5U> reserved0;
    /// Offset: 0x088 (R/W)  Coprocessor Access Control Register
    volatile uint32_t cpacr;
  };

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

  /// System control block address
  static constexpr intptr_t scb_address = 0xE000'ED00UL;

  /// The core interrupts that all cortex m3, m4, m7 processors have
  static constexpr int core_interrupts = 16;

  /// Pointer to Cortex M system control block registers
  static inline auto* scb = reinterpret_cast<scb_registers_t*>(scb_address);

  /// Pointer to Cortex M Nested Vector Interrupt Controller registers
  static inline auto* nvic = reinterpret_cast<nvic_register_t*>(nvic_address);

  /// Pointer to a statically allocated interrupt vector table
  static inline std::span<interrupt_pointer> interrupt_vector_table;

  /**
   * @brief represents an interrupt request number along with helper functions
   * for setting up the interrupt controller registers.
   *
   */
  class irq_t
  {
  public:
    /**
     * @brief construct an irq_t from an int
     *
     */
    constexpr irq_t(int p_irq)
      : m_irq(p_irq)
    {}

    /**
     * @brief copy constructor for irq_t
     *
     */
    constexpr irq_t(irq_t& p_irq)
      : m_irq(p_irq.m_irq)
    {}

    /**
     * @brief operator overload for = int
     *
     * @param p_irq - new irq value to change this irq into
     * @return constexpr irq_t& - reference to this object
     */
    constexpr irq_t& operator=(int p_irq)
    {
      m_irq = p_irq;
      return *this;
    }

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
    [[nodiscard]] constexpr bool default_enabled() const { return m_irq < 0; }

    /**
     * @brief the enable bit for this interrupt resides within one of the 32-bit
     * registers within the "iser" and "icer" arrays. This function will return
     * the index of which 32-bit register contains the enable bit.
     *
     * @return constexpr uint32_t - array index
     */
    [[nodiscard]] constexpr uint32_t register_index() const
    {
      return static_cast<uint32_t>(m_irq) >> index_position;
    }
    /**
     * @brief return a mask with a 1 bit in the enable position for this irq_t.
     *
     * @return constexpr uint32_t - enable mask
     */
    [[nodiscard]] constexpr uint32_t enable_mask() const
    {
      return 1U << (static_cast<uint32_t>(m_irq) & enable_mask_code);
    }
    /**
     * @brief
     *
     * @return constexpr size_t
     */
    [[nodiscard]] constexpr size_t vector_index() const
    {
      return m_irq + core_interrupts;
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
      const size_t last_irq = (interrupt_vector_table.size() - core_interrupts);
      return std::cmp_greater(m_irq, -core_interrupts) &&
             std::cmp_less(m_irq, last_irq);
    }

  private:
    int m_irq = 0;
  };

  /// Place holder interrupt that performs no work
  static void nop() {}

  /**
   * @brief Initializes the interrupt vector table.
   *
   * This template function does the following:
   * - Statically allocates a 64-byte alligned an interrupt vector table the
   *   size of VectorCount.
   * - Set the default handlers for all interrupt vectors to the "nop" function
   *   which does nothing
   * - Set interrupt_vector_table span to the statically allocated vector table.
   * - Finally it relocates the system's interrupt vector table away from the
   *   hard coded vector table in ROM/Flash memory to the statically allocated
   *   table in RAM.
   *
   * This function should only be called once in the life time of the
   * application, perferably at or near the start of main before any other
   * peripherals that may need the interrupt vector table are initialized.
   * Calling this multiple times can have a multitude of effects and undefined
   * behavior.
   *
   * Calling this function with differing VectorCount values will result in
   * multiple statically allocated interrupt vector tables, which will simply
   * waste space in RAM.
   *
   * @tparam VectorCount - the number of interrupts available for this system
   */
  template<size_t VectorCount>
  static void initialize()
  {
    if constexpr (embed::config::is_a_test()) {
      setup_for_unittesting();
    }

    // Statically allocate a buffer of vectors to be used as the new IVT.
    static constexpr size_t total_vector_count = VectorCount + core_interrupts;
    alignas(64) static std::array<interrupt_pointer, total_vector_count>
      vector_buffer{};

    // Will fill the interrupt handler and vector table with a function that
    // does nothing.
    std::fill(vector_buffer.begin(), vector_buffer.end(), nop);

    // Assign the statically allocated vector within this scope to the global
    // interrupt_vector_table span so that it can be accessed in other
    // functions. This is valid because the interrupt vector table has static
    // storage duration and will exist throughout the duration of the
    // application.
    interrupt_vector_table = vector_buffer;

    // Relocate the interrupt vector table the vector buffer. By default this
    // will be set to the address of the start of flash memory for the MCU.
    scb->vtor = reinterpret_cast<intptr_t>(vector_buffer.data());
  }

  /**
   * @brief Get a reference to interrupt vector table object
   *
   * @return const auto& - interrupt vector table
   */
  static const auto& get_interrupt_vector_table()
  {
    return interrupt_vector_table;
  }

  /**
   * @brief Construct a new interrupt object
   *
   * @param p_irq - interrupt to configure
   */
  explicit interrupt(irq_t p_irq)
    : m_irq(p_irq)
  {
    if constexpr (embed::config::is_a_test()) {
      setup_for_unittesting();
    }
  }

  /**
   * @brief Setup system registers for unit testing
   *
   */
  static void setup_for_unittesting()
  {
    // Dummy registers for unit testing
    static scb_registers_t dummy_scb{};
    static nvic_register_t dummy_nvic{};

    // Replace the address of the scb and nvic pointers with the dummystructures
    // so that they can be inspected during unit tests.
    scb = &dummy_scb;
    nvic = &dummy_nvic;
  }

  /**
   * @brief enable interrupt and set the service routine handler.
   *
   * @param p_handler - the interrupt service routine handler to be executed
   * when the hardware interrupt is fired.
   * @return true - successfully installed handler and enabled interrupt
   * @return false - irq value is outside of the bounds of the table
   */
  bool enable(interrupt_pointer p_handler)
  {
    // IRQ must be between -16 < irq < last_irq
    if (!m_irq.is_valid()) {
      return false;
    }

    interrupt_vector_table[m_irq.vector_index()] = p_handler;

    if (!m_irq.default_enabled()) {
      nvic_enable_irq();
    }
    return true;
  }

  /**
   * @brief disable interrupt and set the service routine handler to "nop".
   *
   * @return true - successfully disabled interrupt
   * @return false - irq value is outside of the bounds of the table
   */
  bool disable()
  {
    // IRQ must be between -16 < irq < last_irq
    if (!m_irq.is_valid()) {
      return false;
    }

    interrupt_vector_table[m_irq.vector_index()] = nop;

    if (!m_irq.default_enabled()) {
      nvic_disable_irq();
    }
    return true;
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
  bool verify_vector_enabled(interrupt_pointer p_handler)
  {
    // Return early if the irq isn't even valid
    if (!m_irq.is_valid()) {
      return false;
    }

    // Check if the handler match
    auto irq_handler = interrupt_vector_table[m_irq.vector_index()];
    bool handlers_are_the_same = (irq_handler == p_handler);

    if (!handlers_are_the_same) {
      return false;
    }

    if (m_irq.default_enabled()) {
      return true;
    }

    uint32_t enable_register = nvic->iser.at(m_irq.register_index());
    return (enable_register & m_irq.enable_mask()) == 0U;
  }

private:
  /**
   * @brief Enables a device-specific interrupt in the NVIC interrupt
   * controller.
   *
   */
  void nvic_enable_irq()
  {
    auto* interrupt_enable = &nvic->iser.at(m_irq.register_index());
    *interrupt_enable = *interrupt_enable | m_irq.enable_mask();
  }

  /**
   * @brief Disables a device-specific interrupt in the NVIC interrupt
   * controller.
   *
   */
  void nvic_disable_irq()
  {
    auto* interrupt_clear = &nvic->icer.at(m_irq.register_index());
    *interrupt_clear = *interrupt_clear | m_irq.enable_mask();
  }

  irq_t m_irq;
};
}  // namespace embed::cortex_m
