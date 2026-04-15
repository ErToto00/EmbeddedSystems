#include <xc.h>
#include "timer.h"

int main(void)
{
    ANSELA = 0x0000;
    ANSELB = 0x0000;
    ANSELC = 0x0000;
    ANSELD = 0x0000;
    ANSELE = 0x0000;
    ANSELG = 0x0000;
    
    TRISAbits.TRISA0 = 0;   // LED1 output
    TRISGbits.TRISG9 = 0;   // LED2 output
   
    TRISEbits.TRISE8 = 1;   // T2 input

    LATAbits.LATA0 = 0;
    LATGbits.LATG9 = 0;

    RPINR0bits.INT1R = 88;

    INTCON2bits.INT1EP = 1;

    IFS1bits.INT1IF = 0;    // clear flag
    IEC1bits.INT1IE = 1;    // enable interrupt

    INTCON2bits.GIE = 1;    // global enable

    tmr_setup_period(TIMER2, 500);

    while (1)
    {
        // LED1 senza interrupt
        LATAbits.LATA0 = !LATAbits.LATA0;
        tmr_wait_ms(TIMER2, 500);
    }

    return 0;
}

void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void)
{
    IFS1bits.INT1IF = 0;   // clear flag
    LATGbits.LATG9 = !LATGbits.LATG9;   // toggle LED2
}