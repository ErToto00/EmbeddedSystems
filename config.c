#include "config.h"
#include "timer.h"
#define CS_ACC LATDbits.LATD6

void input_config(){
    ANSELA = 0x0000;
    ANSELB = 0x0000;
    ANSELC = 0x0000;
    ANSELD = 0x0000;
    ANSELE = 0x0000;
    ANSELG = 0x0000;
}

void uart_config(){
    TRISDbits.TRISD11 = 1;
    TRISDbits.TRISD0 = 0;

    RPINR18bits.U1RXR = 75;
    RPOR0bits.RP64R = 0x01;

    U1MODEbits.BRGH = 0;
    // U1BRG = 468; per baud rate 9600
    U1BRG = 38 // per baud rate 115200

    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
    
    // Interrupt
    IPC2bits.U1RXIP = 5;  
    IFS0bits.U1RXIF = 0;  // Reset flag
    IEC0bits.U1RXIE = 1;  // Enable interrupt

    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
}

void spi_config(){
    TRISAbits.TRISA1 = 1;
    TRISFbits.TRISF12 = 0;
    TRISFbits.TRISF13 = 0;

    RPINR20bits.SDI1R = 0b0010001;
    RPOR12bits.RP109R = 0b000101;
    RPOR11bits.RP108R = 0b000110;

    SPI1CON1bits.MSTEN = 1;
    SPI1CON1bits.MODE16 = 0;
    SPI1CON1bits.CKP = 1;

    SPI1CON1bits.PPRE = 3;
    SPI1CON1bits.SPRE = 3;

    SPI1STATbits.SPIROV = 0;
    SPI1STATbits.SPIEN = 1;
}

#define CS_ACC LATDbits.LATD6

void init_accelerometer() {
    // Sleep mode
    CS_ACC = 0;
    spi_write(0x4B); 
    spi_write(0x01); // Sleep
    CS_ACC = 1;
    tmr_wait_ms(TIMER1, 3); // 3 ms wait 

    // Active mode
    CS_ACC = 0;
    spi_write(0x4C); 
    spi_write(0x00); // Active
    CS_ACC = 1;
    tmr_wait_ms(TIMER1, 10);
}

void set_bandwidth(int bw_val) {
    CS_ACC = 0;
    spi_write(0x41);  
    spi_write(bw_val); 
    CS_ACC = 1;
}