#ifndef ES_LIB_H
#define ES_LIB_H

#include <xc.h>
#include <stdint.h>

// =============================================
// Macros
// =============================================
#define TIMER1 1
#define TIMER2 2

// =============================================
// Function Prototypes
// =============================================

void set_default();

// Timer Functions
void tmr_setup_period(int timer, int ms);
int tmr_wait_period(int timer);
void tmr_wait_ms(int timer, int ms);

#endif /* ES_LIB_H */
