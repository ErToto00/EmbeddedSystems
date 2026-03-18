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

//Interrupt Functions
extern int count_T1;
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void);
void __attribute__((interrupt, auto_psv)) _INT2Interrupt(void);

#endif /* ES_LIB_H */
