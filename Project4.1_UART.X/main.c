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
    
    TRISDbits.TRISD11 = 1; // RPI75 is RD11 (input)
    TRISDbits.TRISD0 = 0; // RP64 is RD0 (output)
            
    RPINR19bits.U2RXR = 75;
    RPOR0bits.RP64R = 3;
    
    // enable UART
    U2BRG = 11; // (7372800 / 4) / (16 ? 9600)? 1
    U2MODEbits.UARTEN = 1; // enable UART
    U2STAbits.UTXEN = 1; // enable U2TX (must be after UARTEN)
    
    U2TXREG = 'C'; // send ?C?

    while (1)
    {
        if (U2STAbits.OERR)
            U2STAbits.OERR = 0; // clear flag for overflow 
        
        while (U2STAbits.URXDA == 0); // nothing to read -> wait
        char c = U2RXREG;             // message received  
        
        while (U2STAbits.UTXBF == 1);    // buffer full -> wait 
        U2TXREG = c;                    // send back character 
    }

    return 0;
}
