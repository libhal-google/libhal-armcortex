#pragma once

#include <libarmcortex/platform/core_cm4.h>

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <libcore/platform/ram.hpp>
#include <libcore/utility/ansi_terminal_codes.hpp>
#include <libcore/utility/time/time.hpp>

namespace sjsu
{
extern void InitializePlatform();
}  // namespace sjsu

extern "C"
{
  void GetRegistersFromStack(uint32_t * fault_stack_address)
  {
    // These are volatile to try and prevent the compiler/linker optimizing them
    // away as the variables never actually get used.  If the debugger won't
    // show the values of the variables, make them global my moving their
    // declaration outside of this function.
    volatile uint32_t r0  = fault_stack_address[0];
    volatile uint32_t r1  = fault_stack_address[1];
    volatile uint32_t r2  = fault_stack_address[2];
    volatile uint32_t r3  = fault_stack_address[3];
    volatile uint32_t r12 = fault_stack_address[4];
    // Link register.
    volatile uint32_t lr = fault_stack_address[5];
    // Program counter.
    volatile uint32_t pc = fault_stack_address[6];
    // Program status register.
    volatile uint32_t psr = fault_stack_address[7];

    printf(SJ2_BACKGROUND_RED
           "Hard Fault Exception Occurred!\n" SJ2_COLOR_RESET);
    printf("r0: 0x%08" PRIX32 ", r1: 0x%08" PRIX32
           ", "
           "r2: 0x%08" PRIX32 ", r3: 0x%08" PRIX32 "\n",
           r0,
           r1,
           r2,
           r3);
    printf("r12: 0x%08" PRIX32 ", lr: 0x%08" PRIX32
           ", "
           "pc: 0x%08" PRIX32 ", psr: 0x%08" PRIX32 "\n",
           r12,
           lr,
           pc,
           psr);

    // When the following line is hit, the variables contain the register values
    // Use a JTAG debugger to inspect these variables
    sjsu::Halt();
  }

  /// Hard Fault, all classes of Fault
  inline void ArmHardFaultHandler(void)
  {
    if constexpr (!sjsu::build::IsPlatform("host"))
    {
      __asm volatile(
          " tst lr, #4                                          \n"
          " ite eq                                              \n"
          " mrseq r0, msp                                       \n"
          " mrsne r0, psp                                       \n"
          " ldr r1, [r0, #24]                                   \n"
          " ldr r2, handler2_address_const                      \n"
          " bx r2                                               \n"
          " handler2_address_const: .word GetRegistersFromStack \n");
    }
  }

  // Declare __libc_init_array function which will be used to run the C++
  // constructors. This is supplied by the compiler, no need to define it
  // yourself.
  // NOLINTNEXTLINE(readability-identifier-naming)
  extern void __libc_init_array(void);

  // The entry point for the application.
  // main() is the entry point for newlib based applications
  extern int main();

  // External declaration for the pointer to the stack top from the linker
  // script
  extern void StackTop(void);

  // Reset entry point for your code.
  // Sets up a simple runtime environment and initializes the C/C++ library.
  inline void ArmResetHandler()
  {
    // External declaration for the pointer to the stack top from the linker
    // script
    // The Hyperload bootloader takes up stack space to execute. The Hyperload
    // bootloader function launches this ISR manually, but it never returns thus
    // it never cleans up the memory it uses. To get that memory back, we have
    // to manually move the stack pointers back to the top of stack.
    const uint32_t kTopOfStack = reinterpret_cast<intptr_t>(&StackTop);
    sjsu::cortex::__set_PSP(kTopOfStack);
    sjsu::cortex::__set_MSP(kTopOfStack);

    // Initialize the FPU.
    // Must be done before any floating point instructions are executed. On
    // ARM processors without FPU, like cortex M3, this operation will do
    // nothing.
    {
      using sjsu::cortex::SCB_Type;
      SCB->CPACR = (SCB->CPACR | ((0b11 << 20) |   // set CP10 Full Access
                                  (0b11 << 22)));  // set CP11 Full Access
    }


    sjsu::InitializeDataSection();
    sjsu::InitializeBssSection();

    // Checks at compile time if this is a unit test. If it is, then do not
    // attempt to run `__libc_init_array()`
    if constexpr (!sjsu::build::IsPlatform("host"))
    {
      // Initialisation C++ libraries
      __libc_init_array();
    }

    sjsu::InitializePlatform();

    // Call main and start program
    int return_code = main();

    // Loop here forever or until a reset or an intterupt occurs.
    exit(return_code);
  }

  /// Non-Masktable Interrupt, this can not be stopped, preempted or prevented
  inline void ArmNMIHandler(void)
  {
    /* Not implemented yet */
  }

  /// Memory Management, MPU mismatch, including Access Violation and No Match
  inline void ArmMemoryManageHandler(void)
  {
    /* Not implemented yet */
  }

  /// Bus Fault, Pre-Fetch- Memory Access Fault, other address/memory related
  /// Fault
  inline void ArmBusFaultHandler(void)
  {
    /* Not implemented yet */
  }

  /// Usage Fault, i.e. Undefined Instructions, Illegal State Transitions
  inline void ArmUsageFaultHandler(void)
  {
    /* Not implemented yet */
  }

  /// Usage Fault, i.e. Undefined Instructions, Illegal State Transitions
  inline void ArmSystemTickHandler(void)
  {
    /* Not implemented yet */
  }
}  // extern "C"
