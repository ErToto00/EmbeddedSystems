/*
 * File:   Function1.c
 * Author: matte
 *
 * Created on 28 febbraio 2026, 16.41
 */

#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <Functions.h>

void Function1(){
    
    //set pin as output
    TRISAbits.TRISA0=0;
    //set led high
    LATAbits.LATA0=1;
    while(1);
    
    return 0; 
}

void Function2(){
    //set RA0 as output
    TRISAbits.TRISA0=0;
    //set T2 as input
    TRISEbits.TRISE8=1;
    
    while(1){
        if(PORTEbits.RE8 == 0)
            LATAbits.LATA0=1;
        else
            LATAbits.LATA0=0;
    }
    
}

void Function3(){
    //set RA0 as output
    TRISAbits.TRISA0=0;
    //set T2 as input
    TRISEbits.TRISE8=1;
    
    while(1){
        if(PORTEbits.RE8 == 0)
            LATAbits.LATA0 = !LATAbits.LATA0;
    }
    
    
}
