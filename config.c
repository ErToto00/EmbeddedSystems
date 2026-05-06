#include "config.h"
#include <xc.h>

// Definizione Chip Select per funzioni interne
#define CS_ACC LATBbits.LATB3

void input_config(){
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
}

void uart_config(){
    TRISDbits.TRISD11 = 1; // RX
    TRISDbits.TRISD0 = 0;  // TX
    RPINR18bits.U1RXR = 75;
    RPOR0bits.RP64R = 0x01;
    U1MODEbits.BRGH = 0;
    U1BRG = 468; 
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1;
}

void spi_config(){
    TRISAbits.TRISA1 = 1;  // MISO
    TRISFbits.TRISF12 = 0; // SCK
    TRISFbits.TRISF13 = 0; // MOSI
    RPINR20bits.SDI1R = 17;
    RPOR12bits.RP109R = 5;
    RPOR11bits.RP108R = 6;
    SPI1CON1bits.MSTEN = 1;
    SPI1CON1bits.MODE16 = 0;
    SPI1CON1bits.CKP = 1;
    SPI1CON1bits.PPRE = 3;
    SPI1CON1bits.SPRE = 3;
    SPI1STATbits.SPIEN = 1;
}

void init_accelerometer() {
    CS_ACC = 0;
    spi_write(0x0F); 
    spi_write(0x03); 
    CS_ACC = 1;
}

void set_bandwidth(int bw_val) {
    CS_ACC = 0;
    spi_write(0x10);  
    spi_write(bw_val); 
    CS_ACC = 1;
}