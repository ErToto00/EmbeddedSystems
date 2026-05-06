#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> 
#include "config.h"
#include "timer.h"

// Circular buffer for UART1 RX
#define BUF_SIZE 64
volatile char buffer[BUF_SIZE];
volatile int head = 0;
volatile int tail = 0;

// Chip Select for Accelerometer (RD6 from your exercises)
#define CS_ACC LATDbits.LATD6

// Global variables for the assignment tasks
int freq = 10;      // Transmission frequency (yy Hz)
int bandwidth = 15; // Filter bandwidth (xx BW)
int ax, ay, az;     // Accelerometer data

void algorithm() {
    tmr_wait_ms(TIMER2, 7);
}

// UART1 RX Interrupt - Stores characters in a circular buffer
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    if (U1STAbits.OERR) U1STAbits.OERR = 0;
    while (U1STAbits.URXDA) {
        char c = U1RXREG;
        int next = (head + 1) % BUF_SIZE;
        if (next != tail) {
            buffer[head] = c;
            head = next;
        }
    }
    IFS0bits.U1RXIF = 0;
}

// SPI Write/Read function (from your exercises)
unsigned int spi_write(unsigned int data) {
    while (SPI1STATbits.SPITBF == 1);
    SPI1BUF = data;
    while (SPI1STATbits.SPIRBF == 0);
    return SPI1BUF;
}

// Simple function to send a string via UART
void send_string(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        while (U1STAbits.UTXBF); // Wait for TX buffer to be empty
        U1TXREG = str[i];
    }
}

int main(void) {
    // Hardware configurations
    input_config();
    uart_config();
    spi_config();
    
    TRISGbits.TRISG9 = 0;   // Led2 output 
    TRISDbits.TRISD6 = 0;   // Chip select    
    CS_ACC = 1; // Inactive high

    // Initial accelerometer setup
    init_accelerometer();
    set_bandwidth(15); // Initial BW of 1000 Hz
    
    tmr_setup_period(TIMER1, 10); // Main loop period: 10ms (100Hz)
    
    int blink_counter = 0;
    int main_counter = 0;
    char msg[20];
    int msg_idx = 0;
    char out_buf[60];

    while (1) {
        algorithm();

        // Read UART 
        while (head != tail) {
            char c = buffer[tail];
            tail = (tail + 1) % BUF_SIZE;
            
            if (c == '$') msg_idx = 0;
            msg[msg_idx++] = c;
            
            if (c == '*') {
                msg[msg_idx] = '\0';
                // Handle $HZ,yy* command
                if (msg[1] == 'H' && msg[2] == 'Z') {
                    int val = atoi(&msg[4]);
                    if (val == 0 || val == 1 || val == 2 || val == 5 || val == 10) freq = val;
                    else send_string("$ERR,1*\r\n");
                } 
                // Handle $BW,xx* command
                else if (msg[1] == 'B' && msg[2] == 'W') {
                    int val = atoi(&msg[4]);
                    if (val >= 8 && val <= 15) {
                        bandwidth = val;
                        set_bandwidth(bandwidth);
                    } else send_string("$ERR,1*\r\n");
                }
                msg_idx = 0;
            }
        }

        // Accelerometer aquisition 
        // Runs every 2 loops (2 * 10ms = 20ms)
        if (main_counter % 2 == 0) {
            unsigned int lsb, msb;
            CS_ACC = 0;
            
            spi_write(0x02 | 0x80); // Read starting from X_LSB (0x02)
            lsb = spi_write(0x00); msb = spi_write(0x00); // X axis
            ax = (int)((msb << 8) | (lsb & 0xF8)) >> 3;
            
            lsb = spi_write(0x00); msb = spi_write(0x00); // Y axis
            ay = (int)((msb << 8) | (lsb & 0xF8)) >> 3;
            
            lsb = spi_write(0x00); msb = spi_write(0x00); // Z axis
            az = (int)((msb << 8) | (lsb & 0xF8)) >> 3;
            
            CS_ACC = 1;
        }

        // Send acc data 
        // Calculates the required interval based on freq variable
        if (freq > 0 && (main_counter % (100 / freq) == 0)) {
            sprintf(out_buf, "$ACC,%d,%d,%d*\r\n", ax, ay, az);
            send_string(out_buf);
        }

        // Compute and send angles 
        // Runs every 20 loops (20 * 10ms = 200ms)
        if (main_counter % 20 == 0) {
            double roll = atan2(ay, az) * 57.2958;
            double pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 57.2958;
            sprintf(out_buf, "$ANG,%.1f,%.1f*\r\n", roll, pitch);
            send_string(out_buf);
        }

        // Blink led2
        // No busy-waiting: toggles every 500ms (50 cycles)
        blink_counter++;
        if (blink_counter >= 50) {
            LATGbits.LATG9 = !LATGbits.LATG9;
            blink_counter = 0;
        }

        // Counter management to synchronize frequencies
        main_counter++;
        if (main_counter >= 100) main_counter = 0;
        
        // Wait for the end of the 10ms period
        tmr_wait_period(TIMER1);
    }
}