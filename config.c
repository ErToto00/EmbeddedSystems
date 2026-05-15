#include "config.h"
#include <xc.h>

#define CS_ACC LATBbits.LATB3

// Input as digital
// Inserisci queste definizioni in alto
#define IR_EN_TRIS TRISGbits.TRISG1 // Pin EN del sensore IR (RG1)
#define IR_EN_LAT  LATGbits.LATG1

void input_config() {
    // Set all pins as digital
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    // Battery: AN11 is on RB11
    TRISBbits.TRISB11 = 1; 
    ANSELB |= 0x0800; // Set bit as analog

    // IR sensor: AN2 is on RB2
    TRISBbits.TRISB2 = 1;
    ANSELB |= 0x0004; // Set bit as analog 
    
    // Eneable IR sensor 
    IR_EN_TRIS = 0;
    IR_EN_LAT = 1;
}

void uart_config() {
    TRISDbits.TRISD11 = 1; // RX pin
    TRISDbits.TRISD0 = 0;  // TX pin
    RPINR18bits.U1RXR = 75; // Map U1RX to RP75
    RPOR0bits.RP64R = 0x01; // Map U1TX to RP64
    U1MODEbits.BRGH = 0;    
    U1BRG = 468; 

    // Interrupt configurations
    IPC2bits.U1RXIP = 5;
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1;

    IPC3bits.U1TXIP = 5;
    IFS0bits.U1TXIF = 0;
    IEC0bits.U1TXIE = 1;

    U1MODEbits.UARTEN = 1; // Enable UART
    U1STAbits.UTXEN = 1;  // Enable Transmitter
}

void spi_config() {
    TRISAbits.TRISA1 = 1;  // SDI
    TRISFbits.TRISF12 = 0; // SCK
    TRISFbits.TRISF13 = 0; // SDO
    RPINR20bits.SDI1R = 17;
    RPOR12bits.RP109R = 5;
    RPOR11bits.RP108R = 6;

    SPI1CON1bits.MSTEN = 1;
    SPI1CON1bits.MODE16 = 0;
    SPI1CON1bits.CKP = 1;
    SPI1CON1bits.CKE = 0;
    SPI1CON1bits.PPRE = 3;
    SPI1CON1bits.SPRE = 3;
    SPI1STATbits.SPIEN = 1;
    
    // SPI Chip Select Pins
    TRISDbits.TRISD6 = 0; // MAG
    TRISBbits.TRISB4 = 0; // GYRO
    TRISBbits.TRISB3 = 0; // ACC
    
    // Initialize all CS to High (inactive) 
    LATDbits.LATD6 = 1; 
    LATBbits.LATB4 = 1;
    LATBbits.LATB3 = 1;
}

unsigned int spi_write(unsigned int data) {
    while (SPI1STATbits.SPITBF == 1); // Wait until TX buffer is not full
    SPI1BUF = data;
    while (SPI1STATbits.SPIRBF == 0); // Wait until RX is complete
    return SPI1BUF;
}

void init_accelerometer() {
    CS_ACC = 0;
    spi_write(0x0F); 
    spi_write(0x03); //Sets accelerometer range to ±2g.
    CS_ACC = 1;
}

void set_bandwidth(int bw_val) {
    CS_ACC = 0;
    spi_write(0x10);  
    spi_write(bw_val); 
    CS_ACC = 1;
}

// adc setup (manual/automatic/scan)
void adc_config(int sampling, int conversion, int n_tad, int tad_length, int scan, int scan_mode, int n_channels) {
    AD1CON1bits.ADON = 0;       // Configuration while off
    
    AD1CON1bits.AD12B = 0;      // 10-bits
    AD1CON1bits.FORM = 0;       
    
    AD1CON1bits.ASAM = sampling;   
    AD1CON1bits.SSRC = conversion; 

    if (conversion == 7) {
        if (n_tad < 0) AD1CON3bits.SAMC = 0; 
        else if (n_tad > 31) AD1CON3bits.SAMC = 31;
        else AD1CON3bits.SAMC = n_tad;
        
        if (tad_length < 1) AD1CON3bits.ADCS = 1;
        else if (tad_length > 64) AD1CON3bits.ADCS = 64;
        else AD1CON3bits.ADCS = tad_length;
    }
    
    if (scan == 1) {
        AD1CON2bits.CSCNA = 1;    // Scan mode 
        AD1CON1bits.SIMSAM = 0;   // Sequential sampling 

        if (n_channels > 0)
            AD1CON2bits.SMPI = n_channels - 1; 
        else 
            AD1CON2bits.SMPI = 0;
    } else {
        AD1CON2bits.CSCNA = 0;
        AD1CON2bits.SMPI = 0;
    }
    
    AD1CON1bits.ADON = 1;       // Turn on adc
}