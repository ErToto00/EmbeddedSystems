# ES Custom C Library

This library collects the reusable utility functions shared across the various sub-projects in the "EmbeddedSystems" repository.

## Available Functions

### Initialization
- `void set_default()`: Sets all the analogical input registers (ANSELA to ANSELG) to 0, configuring them as digital I/O.

### Timer Management
- `void tmr_setup_period(int timer, int ms)`: Sets up the specified timer with a period in milliseconds.
- `int tmr_wait_period(int timer)`: Waits for the timer period to complete and resets its flag.
- `void tmr_wait_ms(int timer, int ms)`: Inserts a blocking delay equal to the specified milliseconds using the indicated timer.

## Macros
- `TIMER1`: 1
- `TIMER2`: 2
