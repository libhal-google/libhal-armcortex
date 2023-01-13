#include <libhal-armcortex/dwt_counter.hpp>

int main()
{
  hal::cortex_m::dwt_counter counter(1'000'000.0f);
  return counter.uptime().value();
}
