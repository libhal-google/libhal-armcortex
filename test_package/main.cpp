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

#include <array>
#include <exception>
#include <span>

#include <libhal-armcortex/dwt_counter.hpp>

// Demonstrate function that throws
void foo()
{
  // volatile integer used to keep
  static volatile int a = 0;
  a = a + 1;
  throw 5;
}

int main()
{
  try {
    hal::cortex_m::dwt_counter counter(1'000'000.0f);
    counter.uptime().ticks;
  } catch (...) {
    std::terminate();
  }
  try {
    foo();
  } catch (...) {
    std::terminate();
  }

  return 0;
}

[[noreturn]] void terminate() noexcept
{
  while (true) {
    continue;
  }
}

namespace __cxxabiv1 {                                   // NOLINT
std::terminate_handler __terminate_handler = terminate;  // NOLINT
}

void operator delete(void*)
{
  // Dynamic memory is NOT used in the code, thus any possible call to it should
  // terminate.
  std::terminate();
}

extern "C"
{
  struct _reent* _impure_ptr = nullptr;  // NOLINT
  void _exit([[maybe_unused]] int rc)
  {
    while (true) {
      continue;
    }
  }
  int kill(int, int)
  {
    return -1;
  }
  int getpid()
  {
    return 1;
  }
  std::array<std::uint8_t, 256> storage;
  std::span<std::uint8_t> storage_left(storage);
  void* __wrap___cxa_allocate_exception(unsigned int p_size)  // NOLINT
  {
    // I only know this needs to be 128 because of the disassembly. I cannot
    // figure out why its needed yet, but maybe the answer is in the
    // libunwind-arm.cpp file.
    static constexpr size_t offset = 128;
    if (p_size + offset > storage_left.size()) {
      return nullptr;
    }
    auto* memory = &storage_left[offset];
    storage_left = storage_left.subspan(p_size + offset);
    return memory;
  }
  void __wrap___cxa_free_exception(void*)  // NOLINT
  {
    storage_left = std::span<std::uint8_t>(storage);
  }
  void __wrap___cxa_call_unexpected(void*)  // NOLINT
  {
    std::terminate();
  }
}  // extern "C"
