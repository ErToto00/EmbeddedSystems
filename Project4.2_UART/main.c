#include <xc.h>
#include "es_lib.h"

#define BUF_SIZE 32

volatile char buffer[BUF_SIZE];
volatile int head = 0;
volatile int tail = 0;

void algorithm() {
    tmr_wait_ms(TIMER2, 7);
}

// UART2 RX INTERRUPT
void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void)
{
    if (U2STAbits.OERR)
        U2STAbits.OERR = 0;

    while (U2STAbits.URXDA)
    {
        char c = U2RXREG;

        int next = (head + 1) % BUF_SIZE;

        if (next != tail)
        {
            buffer[head] = c;
            head = next;
        }
    }

    IFS1bits.U2RXIF = 0;
}

int main(void)
{
    ANSELA = 0x0000;
    ANSELB = 0x0000;
    ANSELC = 0x0000;
    ANSELD = 0x0000;
    ANSELE = 0x0000;
    ANSELG = 0x0000;

    TRISAbits.TRISA0 = 0; // LD1
    TRISGbits.TRISG9 = 0; // LD2

    TRISDbits.TRISD11 = 1; // RX
    TRISDbits.TRISD0  = 0; // TX

    LATAbits.LATA0 = 0;
    LATGbits.LATG9 = 0;

    int blink_enable = 1;
    int blink_counter = 0;

    char c1 = 0, c2 = 0, c3 = 0;

    // UART2 mapping
    RPINR19bits.U2RXR = 75;
    RPOR0bits.RP64R = 3;

    // UART2 setup
    U2MODEbits.BRGH = 0;
    U2BRG = 478;

    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN = 1;

    // interrupt RX
    IPC7bits.U2RXIP = 5;
    IFS1bits.U2RXIF = 0;
    IEC1bits.U2RXIE = 1;

    // global interrupt
    INTCON2bits.GIE = 1;

    tmr_setup_period(TIMER1, 10);

    while (1)
    {
        // task 7 ms
        algorithm();

        // LEGGI BUFFER
        while (head != tail)
        {
            char c = buffer[tail];
            tail = (tail + 1) % BUF_SIZE;

            // ECHO
            while (U2STAbits.UTXBF);
            U2TXREG = c;

            // SHIFT per riconoscere stringhe
            c1 = c2;
            c2 = c3;
            c3 = c;

            // "LD1" -> toggle LED1
            if (c1 == 'L' && c2 == 'D' && c3 == '1')
            {
                LATAbits.LATA0 = !LATAbits.LATA0;
            }

            // "LD2" -> stop/resume blink
            if (c1 == 'L' && c2 == 'D' && c3 == '2')
            {
                blink_enable = !blink_enable;
            }
        }

        // BLINK LD2 a 2.5 Hz
        // 10 ms loop -> toggle ogni 20 = 200 ms
        blink_counter++;
        if (blink_counter >= 20)
        {
            blink_counter = 0;

            if (blink_enable)
                LATGbits.LATG9 = !LATGbits.LATG9;
        }

        tmr_wait_period(TIMER1);
    }

    return 0;
}