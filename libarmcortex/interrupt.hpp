#pragma once

#include <cstdint>

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
using interrupt_handler = void (*)(void);

class interrupt
{
public:
  /// NVIC address
  static constexpr intptr_t nvic_address = 0xE000'E100UL;

  /// System control block address
  static constexpr intptr_t scb_address = 0xE000'ED00UL;

  /// The core interrupts that all cortex m3, m4, m7 processors have
  static constexpr int core_interrupt_count = 16;

  /// Pointer to Cortex M system control block registers
  static inline auto* scb = reinterpret_cast<scb_registers_t*>(scb_address);

  /// Pointer to Cortex M Nested Vector Interrupt Controller registers
  static inline auto* nvic = reinterpret_cast<nvic_register_t*>(nvic_address);

  /// Pointer to a statically allocated interrupt vector table
  static inline std::span<interrupt_handler> interrupt_vector_table;

  /// Holds the current_vector that is running
  static inline int current_vector = 0;

  /// Basic interrupt that performs no work
  static void nop() {}

  template<size_t vector_count>
  static void initialize()
  {
    // Statically allocate a buffer of vectors to be used as the new IVT.
    static std::array<interrupt_handler, vector_count> vector_buffer{};

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
  static bool enable(int irq, interrupt_handler handler)
  {
    if (irq >= interrupt_vector_table.size()) {
      return false;
    }

    interrupt_vector_table[irq + core_interrupt_count] = handler;

    if (irq >= 0) {
      nvic_enable_irq(irq);
    }

    return false;
  }

  static bool disable(int irq)
  {
    if (irq < 0) {
      return false;
    }

    nvic_disable_irq(irq);
    interrupt_vector_table[irq + core_interrupt_count] = nop;
  }

  static const auto& get_interrupt_vector_table()
  {
    return interrupt_vector_table;
  }

protected:
  /// Enable External Interrupt
  /// Enables a device-specific interrupt in the NVIC interrupt controller.
  ///
  /// @param irq - External interrupt number. Value cannot be negative.
  static void nvic_enable_irq(int irq)
  {
    unsigned index = irq >> 5;
    unsigned bit_position = irq & 0x1F;

    nvic->ISER[index] = 1 << bit_position;
  }

  /// Disable External Interrupt
  /// Disables a device-specific interrupt in the NVIC interrupt controller.
  ///
  /// @param irq - External interrupt number. Value cannot be negative.
  static void nvic_disable_irq(int irq)
  {
    unsigned index = irq >> 5;
    unsigned bit_position = irq & 0x1F;

    nvic->ICER[index] = 1 << bit_position;
  }
};
} // namespace cortex_m
