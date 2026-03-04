/*
 * File:   main.c
 * Author: matte
 *
 * Created on February 26, 2026, 10:20 AM
 */

#include <xc.h>
#include <stdlib.h>
#include <stdio.h>


int
main( void ){
// all analog pins disabled
ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;

//set pin as output
TRISAbits.TRISA0=0;
//set led high
LATAbits.LATA0=1;
while(1);

return 0;
}