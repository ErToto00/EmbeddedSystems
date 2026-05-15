#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> 
#include "config.h"
#include "timer.h"

// Circular buffers
#define BUF_SIZE 64
volatile char rx_buffer[BUF_SIZE];
volatile int rx_head = 0;
volatile int rx_tail = 0;

#define TX_BUF_SIZE 256 
volatile char tx_buffer[TX_BUF_SIZE];
volatile int tx_head = 0;
volatile int tx_tail = 0;

#define CS_ACC LATBbits.LATB3
#define PIN_IR_EN LATGbits.LATG1
#define TRIS_IR_EN TRISGbits.TRISG1

// Constants for calculations
#define ADC_MAX 1023.0              // 10-bit ADC
#define VREF 3.3                    // Reference Voltage
#define BATTERY_DIVIDER 2.0         // Voltage divider factor

// UART Interrupts
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    if (U1STAbits.OERR) U1STAbits.OERR = 0;
    while (U1STAbits.URXDA) {
        char c = U1RXREG;
        int next = (rx_head + 1) % BUF_SIZE;
        if (next != rx_tail) {
            rx_buffer[rx_head] = c;
            rx_head = next;
        }
    }
    IFS0bits.U1RXIF = 0;
}

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    while (!U1STAbits.UTXBF && tx_tail != tx_head) {
        U1TXREG = tx_buffer[tx_tail];
        tx_tail = (tx_tail + 1) % TX_BUF_SIZE;
    }
    IFS0bits.U1TXIF = 0;
}

void send_string(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        int next = (tx_head + 1) % TX_BUF_SIZE;
        while (next == tx_tail);
        tx_buffer[tx_head] = str[i];
        tx_head = next;
    }
    if (IFS0bits.U1TXIF == 0) IFS0bits.U1TXIF = 1;
}

// Assignment 1: Read battery (Manual/Manual)
float read_battery() {
    adc_config(0, 0, 0, 0, 0, 0, 0); 
    AD1CHS0bits.CH0SA = 11;      // AN11
    
    AD1CON1bits.SAMP = 1;        // Start sampling
    tmr_wait_ms(TIMER2, 2);      // Wait using Timer 2
    AD1CON1bits.SAMP = 0;        // Start conversion
    
    while (!AD1CON1bits.DONE);
    return (ADC1BUF0 / ADC_MAX) * VREF * BATTERY_DIVIDER;
}

// Assignment 2: Read IR sensor (Manual/Auto)
double read_distance() {
    adc_config(0, 7, 16, 8, 0, 0, 0);
    AD1CHS0bits.CH0SA = 2;       // AN2
    
    AD1CON1bits.SAMP = 1;        // Start sampling, auto-conversion
    
    while (!AD1CON1bits.DONE);
    double v_ir = (ADC1BUF0 / ADC_MAX) * VREF;
    
    double distance = 2.34 - 4.74*v_ir + 4.06*pow(v_ir,2) - 1.60*pow(v_ir,3) + 0.24*pow(v_ir,4);
    return distance;
}

int main(void) {
    input_config();
    uart_config();
    spi_config();
    
    // Pins configuration
    ANSELB |= 0x0800;      // AN11
    ANSELB |= 0x0004;      // AN2
    TRISBbits.TRISB11 = 1; 
    TRISBbits.TRISB2 = 1;  
    
    TRIS_IR_EN = 0;
    PIN_IR_EN = 1;
    
    tmr_setup_period(TIMER1, 10); // 10ms loop
    int main_counter = 0;
    char out_buf[100];

    while (1) {
        if (main_counter % 100 == 0) { // Every 1 second
            float battery = read_battery();
            double distance = read_distance();
            sprintf(out_buf, "$BAT,%.2f* $DIST,%.2f*\r\n", (double)battery, distance);
            send_string(out_buf);
        }

        main_counter++;
        if (main_counter >= 1000) main_counter = 0;

        tmr_wait_period(TIMER1);
    }
    return 0;
}