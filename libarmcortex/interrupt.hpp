#pragma once

#include <libembeddedhal/context.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

namespace cortex_m {
/// Structure type to access the System Control Block (SCB).
struct scb_registers_t
{
  /// Offset: 0x000 (R/ )  CPUID Base Register
  const volatile uint32_t CPUID;
  /// Offset: 0x004 (R/W)  Interrupt Control and State Register
  volatile uint32_t ICSR;
  /// Offset: 0x008 (R/W)  Vector Table Offset Register
  volatile uint32_t VTOR;
  /// Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register
  volatile uint32_t AIRCR;
  /// Offset: 0x010 (R/W)  System Control Register
  volatile uint32_t SCR;
  /// Offset: 0x014 (R/W)  Configuration Control Register
  volatile uint32_t CCR;
  /// Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 5)
  volatile uint8_t SHP[12U];
  /// Offset: 0x024 (R/W)  System Handler Control and State Register
  volatile uint32_t SHCSR;
  /// Offset: 0x028 (R/W)  Configurable Fault Status Register
  volatile uint32_t CFSR;
  /// Offset: 0x02C (R/W)  HardFault Status Register
  volatile uint32_t HFSR;
  /// Offset: 0x030 (R/W)  Debug Fault Status Register
  volatile uint32_t DFSR;
  /// Offset: 0x034 (R/W)  MemManage Fault Address Register
  volatile uint32_t MMFAR;
  /// Offset: 0x038 (R/W)  BusFault Address Register
  volatile uint32_t BFAR;
  /// Offset: 0x03C (R/W)  Auxiliary Fault Status Register
  volatile uint32_t AFSR;
  /// Offset: 0x040 (R/ )  Processor Feature Register
  const volatile uint32_t PFR[2U];
  /// Offset: 0x048 (R/ )  Debug Feature Register
  const volatile uint32_t DFR;
  /// Offset: 0x04C (R/ )  Auxiliary Feature Register
  const volatile uint32_t ADR;
  /// Offset: 0x050 (R/ )  Memory Model Feature Register
  const volatile uint32_t MMFR[4U];
  /// Offset: 0x060 (R/ )  Instruction Set Attributes Register
  const volatile uint32_t ISAR[5U];
  uint32_t RESERVED0[5U];
  /// Offset: 0x088 (R/W)  Coprocessor Access Control Register
  volatile uint32_t CPACR;
};

/// Structure type to access the Nested Vectored Interrupt Controller (NVIC)
struct nvic_register_t
{
  /// Offset: 0x000 (R/W)  Interrupt Set Enable Register
  volatile uint32_t ISER[8U];
  uint32_t RESERVED0[24U];
  /// Offset: 0x080 (R/W)  Interrupt Clear Enable Register
  volatile uint32_t ICER[8U];
  uint32_t RSERVED1[24U];
  /// Offset: 0x100 (R/W)  Interrupt Set Pending Register
  volatile uint32_t ISPR[8U];
  uint32_t RESERVED2[24U];
  /// Offset: 0x180 (R/W)  Interrupt Clear Pending Register
  volatile uint32_t ICPR[8U];
  uint32_t RESERVED3[24U];
  /// Offset: 0x200 (R/W)  Interrupt Active bit Register
  volatile uint32_t IABR[8U];
  uint32_t RESERVED4[56U];
  /// Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide)
  volatile uint8_t IP[240U];
  uint32_t RESERVED5[644U];
  /// Offset: 0xE00 ( /W)  Software Trigger Interrupt Register
  volatile uint32_t STIR;
};

/// Used specifically for defining an interrupt vector table of addresses.
using interrupt_pointer = void (*)(void);

class interrupt
{
public:
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

  class irq_t
  {
  public:
    constexpr irq_t(int p_irq)
      : m_irq(p_irq)
    {}

    constexpr irq_t& operator=(int p_irq)
    {
      m_irq = p_irq;
      return *this;
    }

    constexpr unsigned int register_index() { return m_irq >> 5; }
    constexpr unsigned int enable_mask() { return 1 << (m_irq & 0x1F); }
    constexpr unsigned int vector_index() { return m_irq + core_interrupts; }
    constexpr bool default_enabled() { return m_irq < 0; }
    constexpr bool is_valid()
    {
      const int last_irq = interrupt_vector_table.size() - core_interrupts;
      return -core_interrupts < m_irq && m_irq < last_irq;
    }

  private:
    int m_irq = 0;
  };

  /// Basic interrupt that performs no work
  static void nop() {}

  template<size_t vector_count>
  static void initialize()
  {
    if constexpr (embed::is_a_test()) {
      setup_for_unittesting();
    }

    // Statically allocate a buffer of vectors to be used as the new IVT.
    static constexpr int total_vector_count = vector_count + core_interrupts;
    static std::array<interrupt_pointer, total_vector_count> vector_buffer{};

    // Will fill the interrupt handler and vector table with a function that
    // does nothing.
    std::fill(vector_buffer.begin(), vector_buffer.end(), nop);

    // Assign this inner vector to the global interrupt_vector_table span so
    // that it can be accessed in other functions.
    interrupt_vector_table = vector_buffer;

    // Relocate the interrupt vector table the vector buffer. By default this
    // will be set to the address of the start of flash memory for the MCU.
    scb->VTOR = reinterpret_cast<intptr_t>(vector_buffer.data());
  }

  interrupt(irq_t p_irq)
    : m_irq(p_irq)
  {
    if constexpr (embed::is_a_test()) {
      setup_for_unittesting();
    }
  }

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

  /// @brief Enable interrupt base
  ///
  /// @return true if this was successful
  bool enable(interrupt_pointer handler)
  {
    const int last_irq = interrupt_vector_table.size() - core_interrupts;

    // IRQ must be between -16 < irq < last_irq
    if (!m_irq.is_valid()) {
      return false;
    }

    interrupt_vector_table[m_irq.vector_index()] = handler;

    if (!m_irq.default_enabled()) {
      nvic_enable_irq();
    }
    return true;
  }

  bool disable()
  {
    const int last_irq = interrupt_vector_table.size() - core_interrupts;

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

  static const auto& get_interrupt_vector_table()
  {
    return interrupt_vector_table;
  }

  const bool verify_vector_enabled(interrupt_pointer p_handler)
  {
    if (m_irq.is_valid()) {
      bool check_vector, check_is_enabled;
      check_vector = interrupt_vector_table[m_irq.vector_index()] == p_handler;

      if (m_irq.default_enabled()) {
        check_is_enabled = true;
      } else {
        check_is_enabled =
          (nvic->ISER[m_irq.register_index()] & m_irq.enable_mask());
      }

      return check_vector && check_is_enabled;
    }
    return true;
  }

protected:
  /// Enable External Interrupt
  /// Enables a device-specific interrupt in the NVIC interrupt controller.
  ///
  /// @param irq - External interrupt number. Value cannot be negative.
  void nvic_enable_irq()
  {
    auto* interrupt_enable = &nvic->ISER[m_irq.register_index()];
    *interrupt_enable = *interrupt_enable | m_irq.enable_mask();
  }

  /// Disable External Interrupt
  /// Disables a device-specific interrupt in the NVIC interrupt controller.
  ///
  /// @param irq - External interrupt number. Value cannot be negative.
  void nvic_disable_irq()
  {
    auto* interrupt_clear = &nvic->ICER[m_irq.register_index()];
    *interrupt_clear = *interrupt_clear | m_irq.enable_mask();
  }

  irq_t m_irq;
};
} // namespace cortex_m
