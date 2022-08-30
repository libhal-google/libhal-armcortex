#pragma once

#include <array>
#include <cstdint>

#include <libhal/config.hpp>

namespace hal::cortex_m {
/**
 * @brief Driver for controlling and inspect various aspects of the Cortex Mx
 * Systems such as interrupt vector table location, fault address locations and
 * fpu (coprocessor) control.
 *
 */
class system_control
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
    volatile intptr_t vtor;
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
    /// Reserved 0
    std::array<uint32_t, 5U> reserved0;
    /// Offset: 0x088 (R/W)  Coprocessor Access Control Register
    volatile uint32_t cpacr;
  };

  /// System control block address
  static constexpr intptr_t scb_address = 0xE000'ED00UL;

  /// @return auto* - Address of the Cortex M system control block register
  static auto* scb()
  {
    if constexpr (hal::is_a_test()) {
      static scb_registers_t dummy_scb{};
      return &dummy_scb;
    }
    return reinterpret_cast<scb_registers_t*>(scb_address);
  }
  /**
   * @brief Enable the floating point unit coprocessor within Cortex M4 and
   * above processor.
   *
   */
  void initialize_floating_point_unit()
  {
    scb()->cpacr = scb()->cpacr | ((0b11 << 10 * 2) | /* set CP10 Full Access */
                                   (0b11 << 11 * 2)); /* set CP11 Full Access */
  }

  /**
   * @brief Set the address of the systems interrupt vector table
   *
   * The interrupt vector table (IVT) is held in ROM which means that, either
   * the interrupt service routines (ISR) had to be defined at compile time
   * making them immutable at runtime, or that each ISR calls a mutable function
   * pointer which can be changed at runtime.
   *
   * The problem with the first option is that it makes writing and using
   * libraries difficult. Usually requiring updates to the IVT manually by the
   * application designer based on what libraries and drivers the application is
   * using.
   *
   * The second solution has a problem where the additional another layer of
   * indirection increases interrupt latency. A more critical problem of this
   * approach is that many ISRs take advantage of the state of the system when
   * the ISR runs. For example, context switching in an RTOS needs to be able to
   * see the address of where code was when the interrupt occurred and having an
   * additional point of indirection (i.e. calling a function pointer) will
   * change that location from the task to the ISR that called the context
   * switch function. This will usually result in a fault of some sort.
   *
   * Creating an interrupt vector table in RAM and relocating the ISRs there
   * consumes RAM space, but gives great flexibility over the table at runtime.
   *
   * @param p_table_location - address of the interrupt vector table.
   */
  void set_interrupt_vector_table_address(void* p_table_location)
  {
    // Relocate the interrupt vector table the vector buffer. By default this
    // will be set to the address of the start of flash memory for the MCU.
    scb()->vtor = reinterpret_cast<intptr_t>(p_table_location);
  }
  /**
   * @brief Get the address of the systems interrupt vector table.
   *
   * On reset the VTOR register is set to 0x0000'0000 or nullptr.
   *
   * @return void* - address within VTOR the interrupt vector table relocation
   * register.
   */
  void* get_interrupt_vector_table_address()
  {
    // Relocate the interrupt vector table the vector buffer. By default this
    // will be set to the address of the start of flash memory for the MCU.
    return reinterpret_cast<void*>(scb()->vtor);
  }
};
}  // namespace hal::cortex_m
