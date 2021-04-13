#pragma once

#include <libarmcortex/platform/core_cm4.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <libcore/peripherals/interrupt.hpp>
#include <libcore/utility/error_handling.hpp>
#include <span>

namespace sjsu::cortex
{
/// Cortex M interrupt controller
///
/// @tparam kNumberOfInterrupts - the number of interrupts the microcontroller
///         supports.
/// @tparam kNvicPriorityBits - the number of bits dedicated to priority
template <size_t kNumberOfInterrupts, uint32_t kNvicPriorityBits>
class InterruptController final : public sjsu::InterruptController
{
 public:
  /// The number of ARM exceptions before reaching the MCUs
  static constexpr int kArmExceptions = (-cortex::Reset_IRQn) + 1;
  /// Pointer to Cortex M system control block registers
  inline static SCB_Type * scb = SCB;
  /// Pointer to Cortex M Nested Vector Interrupt Controller registers
  inline static NVIC_Type * nvic = NVIC;
  /// Holds the current_vector that is running
  inline static int current_vector = cortex::Reset_IRQn;

  static constexpr size_t kTableSize = kNumberOfInterrupts + kArmExceptions;

  inline static InterruptController * global_controller = nullptr;

  /// @param irq - irq number to convert
  /// @return A convert an irq number into lookup table index
  static int IRQToIndex(int irq)
  {
    return irq + kArmExceptions;
  }

  /// @param index - index to convert
  /// @return A convert a lookup table index into an irq number.
  static int IndexToIRQ(int index)
  {
    return index - kArmExceptions;
  }

  /// This must be put into the interrupt vector table for all of the interrupts
  /// this lookup handler will work for in ROM at compile time.
  /// @note This function should only be called by the processor not by the
  /// application.
  static void LookupHandler()
  {
    int active_interrupt = (scb->ICSR & 0xFF);
    current_vector       = IndexToIRQ(active_interrupt);
    InterruptHandler handler =
        global_controller->external_interrupt_handlers[current_vector];
    handler();
  }

  InterruptController(
      std::span<const sjsu::InterruptVectorAddress> original_ivt)
      : interrupt_vector_table{}, external_interrupt_handlers{}
  {
    // Will fill the interrupt handler and vector table with the default
    // contents.
    std::fill_n(interrupt_vector_table.begin(), kTableSize, LookupHandler);
    std::fill(external_interrupt_handlers.begin(),
              external_interrupt_handlers.end(),
              UnregisteredHandler);

    // Copy the contents of the original interrupt vector table into the
    // object's interrupt_vector_table. On ModuleInitialize() this will become
    // the vector table for the platform.
    std::copy_n(
        original_ivt.begin(),
        std::min(original_ivt.size(), static_cast<size_t>(kArmExceptions)),
        interrupt_vector_table.begin());
  }

  void ModuleInitialize() override
  {
    global_controller = this;
    scb->VTOR = reinterpret_cast<intptr_t>(interrupt_vector_table.data());
  }

  void Enable(RegistrationInfo_t register_info) override
  {
    int irq = register_info.interrupt_request_number;

    if (irq > 0)
    {
      external_interrupt_handlers[irq] = register_info.interrupt_handler;
    }
    else
    {
      interrupt_vector_table[IRQToIndex(irq)] = register_info.interrupt_vector;
    }

    if (irq >= 0)
    {
      NvicEnableIRQ(irq);
    }
    if (register_info.priority > -1)
    {
      NvicSetPriority(irq, register_info.priority);
    }
  }

  void Disable(int interrupt_request_number) override
  {
    if (interrupt_request_number >= 0)
    {
      NvicDisableIRQ(interrupt_request_number);
      external_interrupt_handlers[interrupt_request_number] =
          UnregisteredHandler;
    }
  }

 private:
  alignas(512) std::array<InterruptVectorAddress,
                          kTableSize> interrupt_vector_table = { nullptr };

  std::array<InterruptHandler, kNumberOfInterrupts>
      external_interrupt_handlers = { nullptr };

  /// Enable External Interrupt
  /// Enables a device-specific interrupt in the NVIC interrupt controller.
  ///
  /// @param irq - External interrupt number. Value cannot be negative.
  static void NvicEnableIRQ(int irq)
  {
    nvic->ISER[(irq >> 5)] = (1 << (irq & 0x1F));
  }

  /// Disable External Interrupt
  /// Disables a device-specific interrupt in the NVIC interrupt controller.
  ///
  /// @param irq - External interrupt number. Value cannot be negative.
  static void NvicDisableIRQ(int irq)
  {
    nvic->ICER[(irq >> 5)] = (1 << (irq & 0x1F));
  }

  /// Set Interrupt Priority
  /// Sets the priority of an interrupt.
  /// @note The priority cannot be set for every core interrupt.
  /// @param irq - Interrupt number.
  /// @param priority - Priority to set.
  static void NvicSetPriority(int irq, int priority)
  {
    uint32_t priority_mask = priority << (8U - kNvicPriorityBits);
    if (irq < 0)
    {
      scb->SHP[(irq & 0xFUL) - 4UL] = static_cast<uint8_t>(priority_mask);
    }
    else
    {
      nvic->IP[irq] = static_cast<uint8_t>(priority_mask);
    }
  }

  /// Program will call this if an unexpected interrupt occurs or a specific
  /// handler is not present in the application code.
  static void UnregisteredHandler()
  {
    if (current_vector >= 0)
    {
      NvicDisableIRQ(current_vector);
    }
  }
};
}  // namespace sjsu::cortex
