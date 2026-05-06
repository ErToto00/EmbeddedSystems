#include <xc.h>
#include "timer.h"

// Definizione dei pin Chip Select
#define MAG_CS LATDbits.LATD10
#define IMU_CS LATBbits.LATB14

unsigned int spi_write(unsigned int data) {
    while (SPI1STATbits.SPITBF == 1); // Attende che il buffer di trasmissione sia libero
    SPI1BUF = data;                   // Scrive il dato per iniziare la trasmissione
    while (SPI1STATbits.SPIRBF == 0); // Attende che la ricezione sia completata
    return SPI1BUF;                  // Legge e restituisce il dato ricevuto
}

int main(void) {
    // Impostiamo i CS come output
    TRISDbits.TRISD10 = 0; // Magnetometro
    TRISBbits.TRISB14 = 0; // IMU (Acc/Giro)
    
    // Disattiviamo tutti i chip (Active Low -> Portiamo a 1)
    MAG_CS = 1;
    IMU_CS = 1;

    // UART2 MAPPING E SETUP 
    RPINR19bits.U2RXR = 75; // RX su RP75
    RPOR0bits.RP64R = 3;    // TX su RP64

    U2MODEbits.BRGH = 0;
    U2BRG = 478;            // Baudrate (calcolato per la tua frequenza)
    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN = 1;

    // SPI1 MAPPING E SETUP
    TRISBbits.TRISB8 = 1;     // SDI1 (MISO)
    TRISFbits.TRISF12 = 0; // SCK
    TRISFbits.TRISF13 = 0; // MOSI
    TRISAbits.TRISA1  = 1; // MISO

    SPI1CON1bits.MSTEN = 1;   // Master mode
    SPI1CON1bits.MODE16 = 0;  // 8-bit mode
    SPI1CON1bits.CKP = 1;     // Idle clock high (fondamentale per magnetometro)
    SPI1CON1bits.CKE = 0;     // Sample on transition from idle to active
    SPI1CON1bits.PPRE = 3;    // 1:1 primary prescaler
    SPI1CON1bits.SPRE = 3;    // 5:1 secondary prescaler
    
    SPI1STATbits.SPIEN = 1;   // Abilita SPI

    while (1) {
       tmr_setup_period(TIMER1, 10);
        unsigned int chip_ID = 0;

        // assignment 1
        MAG_CS = 0; // attiviamo il magnetometro in suspend mode 
        spi_write(0x4B);
        spi_write(0x01);
        MAG_CS = 1;

        tmr_wait_ms(TIMER1, 5);

        MAG_CS = 0; // attiviamo il magetometro in active mode 
        spi_write(0x4C);
        spi_write(0x00);
        MAG_CS = 1;

        unsigned int trash;
        MAG_CS = 0;           // Riattivo il Chip Select
        trash = spi_write(0x40 | 0x80); // 1. Invio l'indirizzo 0x40 con bit di lettura (0xC0)
        chip_ID = spi_write(0x00); // 2. Invio uno zero e SALVO quello che il sensore mi ridà
        MAG_CS = 1;           // Chiudo la comunicazion

        if (U2STAbits.OERR)
                U2STAbits.OERR = 0; // clear flag for overflow 

        while (U2STAbits.UTXBF == 1);    // buffer full -> wait 
        U2TXREG = chip_ID;     
    }

    return 0;
}