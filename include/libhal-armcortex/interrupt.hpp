// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <array>
#include <span>
#include <utility>

#include <libhal/error.hpp>

namespace hal::cortex_m {
/// Used specifically for defining an interrupt vector table of addresses.
using interrupt_pointer = void (*)();

/**
 * @brief IRQ numbers for core processor interrupts
 *
 */
enum class irq
{
  top_of_stack = 0,
  reset = 1,
  /// @brief  non-maskable interrupt
  nmi = 2,
  hard_fault = 3,
  memory_management_fault = 4,
  bus_fault = 5,
  usage_fault = 6,
  reserve7 = 7,
  reserve8 = 8,
  reserve9 = 9,
  reserve10 = 10,
  /// @brief Software initiated interrupt
  sv_call = 11,
  reserve12 = 12,
  reserve13 = 13,
  pend_sv = 14,
  systick = 15,
};

/**
 * @brief Cortex M series interrupt controller
 *
 */
class interrupt
{
public:
  /// The core interrupts that all cortex m3, m4, m7 processors have
  static constexpr size_t core_interrupts = 16;

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
     * @param p_id - interrupt request number. If this value is beyond the
     * bounds of the interrupt vector table, meaning it is an invalid exception
     * number, then all operations will do nothing.
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
     * @brief Provides the index within the IVT
     *
     * @return constexpr size_t - the index position
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
    [[nodiscard]] bool is_valid() const
    {
      return m_id < get_vector_table().size();
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

  /// Place holder interrupt that performs no work
  static void nop();

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
    setup(vector_buffer);
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
    reset();
    initialize<VectorCount>();
  }

  /**
   * @brief Get a reference to interrupt vector table object
   *
   * @return const std::span<interrupt_pointer>  - interrupt vector table
   */
  static const std::span<interrupt_pointer> get_vector_table();

  static void disable_interrupts();

  static void enable_interrupts();

  /**
   * @brief Construct a new interrupt object
   *
   * @param p_id - interrupt to configure
   */
  explicit interrupt(exception_number p_id);

  /**
   * @brief enable interrupt and set the service routine handler.
   *
   * @param p_handler - the interrupt service routine handler to be executed
   * when the hardware interrupt is fired.
   */
  void enable(interrupt_pointer p_handler);

  /**
   * @brief disable interrupt and set the service routine handler to "nop".
   *
   * If the IRQ is invalid, then nothing happens.
   */
  void disable();

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
  [[nodiscard]] bool verify_vector_enabled(interrupt_pointer p_handler);

private:
  static void reset();
  static void setup(std::span<interrupt_pointer> p_vector_table);

  exception_number m_id;
};
}  // namespace hal::cortex_m
