#pragma once

namespace hal::cortex_m {
template<typename T>
class stub_out_registers
{
public:
  stub_out_registers(T** p_register_pointer)
    : m_register_pointer(p_register_pointer)
    , m_original(nullptr)
    , m_stub{}
  {
    m_original = *m_register_pointer;
    *m_register_pointer = &m_stub;
  }

  ~stub_out_registers()
  {
    *m_register_pointer = m_original;
  }

private:
  T** m_register_pointer;
  T* m_original;
  T m_stub;
};
}  // namespace hal::cortex_m
