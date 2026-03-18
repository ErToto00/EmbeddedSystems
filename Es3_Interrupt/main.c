/* 
 * File:   main.c
 * Author: Ary
 *
 * Created on March 5, 2026, 10:54 AM
 */

#include <xc.h>
#include "es_lib.h"

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
   
    TRISEbits.TRISE8 = 1;   // T2 as input 

    LATAbits.LATA0 = 0;     // LEDS initially off
    LATGbits.LATG9 = 0;     
    
    RPINR1bits.INT2R = 0x58; // Mappa INT2 al pin RE8 (0x58)
    INTCON2bits.INT2EP = 1;  // Interrupt scatta sul fronte di discesa (pressione T2 che va a GND)
    INTCON2bits.GIE = 1;
    IFS1bits.INT2IF = 0;    // clear interrupt flag
    IEC1bits.INT2IE = 1;    // enable
        
    tmr_setup_period(TIMER2, 500);

    while (1) {
        // L1 lampeggia indipendentemente usando il timer (polling blocccante)
        LATAbits.LATA0 = !LATAbits.LATA0;   // toggle LED
        tmr_wait_ms(TIMER2, 500);
        
        // Non serve più fare il polling (if) per T2!
        // Quando premi il pulsante, l'hardware interromperà il timer
        // in tempo reale e lancerà la funzione _INT2Interrupt(), 
        // che cambierà lo stato del LED 2 (LATG9).
    }
    
    return 0;
}

void __attribute__((interrupt, auto_psv))_INT2Interrupt(){
    IFS1bits.INT2IF = 0;
    LATGbits.LATG9 = !LATGbits.LATG9;
}