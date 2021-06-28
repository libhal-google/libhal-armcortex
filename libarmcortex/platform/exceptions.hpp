#pragma once

#include <libarmcortex/platform/core_cm4.h>

#include <cinttypes>
#include <cstdint>
#include <libcore/platform/ram.hpp>
#include <libcore/platform/startup.hpp>
#include <libcore/utility/ansi_terminal_codes.hpp>
#include <libcore/utility/log.hpp>
#include <libcore/utility/time/time.hpp>

extern "C"
{
  /// Hard Fault, all classes of Fault
  inline void ArmHardFaultHandler(uint32_t stack[])
  {
    if constexpr (!sjsu::build::IsPlatform("host"))
    {
      using sjsu::cortex::SCB_Type;
      enum
      {
        r0,
        r1,
        r2,
        r3,
        r12,
        lr,
        pc,
        psr,
      };

      sjsu::log::Print("In Hard Fault Handler\n");
      sjsu::log::Print("SCB->HFSR = 0x%08lx\n", SCB->HFSR);
      if ((SCB->HFSR & (1 << 30)) != 0)
      {
        sjsu::log::Print("Forced Hard Fault\n");
        sjsu::log::Print("SCB->CFSR = 0x%08lx\n", SCB->CFSR);
        if ((SCB->CFSR & 0xFFFF0000) != 0)
        {
          // printUsageErrorMsg(SCB->CFSR);
        }
      }

      sjsu::log::Print(
          "r0  = 0x%08lx\n"
          "r1  = 0x%08lx\n"
          "r2  = 0x%08lx\n"
          "r3  = 0x%08lx\n"
          "r12 = 0x%08lx\n"
          "lr  = 0x%08lx\n"
          "pc  = 0x%08lx\n"
          "psr = 0x%08lx\n",
          stack[r0],
          stack[r1],
          stack[r2],
          stack[r3],
          stack[r12],
          stack[lr],
          stack[pc],
          stack[psr]);
      __asm volatile("BKPT #01");
      while (1)
      {
        continue;
      }
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
  extern void _stack_top(void);

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
    const uint32_t kTopOfStack = reinterpret_cast<intptr_t>(&_stack_top);
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
