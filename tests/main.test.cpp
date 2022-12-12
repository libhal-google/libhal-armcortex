namespace hal::cortex_m {
extern void dwt_test();
extern void systick_timer_test();
extern void interrupt_test();
}  // namespace hal::cortex_m

int main()
{
  // [Position Dependent Test]:
  // Initializes interrupt vector table and thus must go first
  hal::cortex_m::interrupt_test();
  hal::cortex_m::dwt_test();
  hal::cortex_m::systick_timer_test();
}
