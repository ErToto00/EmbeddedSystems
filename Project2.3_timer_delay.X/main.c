/* 
 * File:   main.c
 * Author: Ary
 *
 * Created on March 5, 2026, 10:54 AM
 */

#include <xc.h>
#include "timer.h"

int main(void)
{
    TRISAbits.TRISA0 = 0;   // L1 output
    TRISGbits.TRISG9 = 0;    // L2 output

    LATAbits.LATA0 = 0;     // LEDS initially off
    LATGbits.LATG9 = 0; 

    int ret = 0;
    tmr_setup_period(TIMER1, 200);
    int delay = 50; // 50,200,2000

    while (1) {
        
        tmr_wait_ms(TIMER2, delay);
        LATAbits.LATA0 = !LATAbits.LATA0;   // toggle LED
        
        ret = tmr_wait_period(TIMER1);
        if(ret)
        {
            LATGbits.LATG9 = 1;
        }else{
            LATGbits.LATG9 = 0;
        }
    }

    return 0;
}
