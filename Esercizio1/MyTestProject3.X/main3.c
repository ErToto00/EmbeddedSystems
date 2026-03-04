/*
 * File:   main3.c
 * Author: matte
 *
 * Created on February 28, 2026, 5:35 PM
 */

#include <xc.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    //set RA0 as output
    TRISAbits.TRISA0=0;
    //set T2 as input
    TRISEbits.TRISE8=1;
    
    while(1){
        
        if(PORTEbits.RE8 == 0)
           LATAbits.LATA0 = !LATAbits.LATA0;
        
    }
    
    return 0;
}
