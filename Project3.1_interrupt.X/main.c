/* 
 * File:   main.c
 * Author: Ary
 *
 * Created on March 5, 2026, 10:54 AM
 */

#include <xc.h>
#include "timer.h"

int count_T1 =  0;

int main(void)
{
    ANSELA = 0x0000;
    ANSELB = 0x0000;
    ANSELC = 0x0000;
    ANSELD = 0x0000;
    ANSELE = 0x0000;
    ANSELG = 0x0000;
    
    TRISAbits.TRISA0 = 0;   // L1 output
    TRISGbits.TRISG9 = 0;   // L2 output

    LATAbits.LATA0 = 0;     // LEDS initially off
    LATGbits.LATG9 = 0; 
    
    tmr_setup_period(TIMER2, 500);
    
    tmr_setup_period(TIMER1, 50);
    
    IFS0bits.T1IF = 0; 
    IEC0bits.T1IE = 1; //enable 

    while (1) {
        // led1 no interrupt
        LATAbits.LATA0 = !LATAbits.LATA0;   // toggle LED
        tmr_wait_ms(TIMER2, 500);
    }
    
    return 0;
}

void __attribute__((interrupt, auto_psv))_T1Interrupt(){
    IFS0bits.T1IF = 0;
    
    count_T1 = count_T1 +1;
    
    // logic with tmr_wait_ms
    if (count_T1 == 5) {   // 5 * 50 ms = 250 ms
        LATGbits.LATG9 = !LATGbits.LATG9;
        count_T1 = 0;
    }
}
