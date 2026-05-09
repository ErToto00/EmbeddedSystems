#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> 
#include "config.h"
#include "timer.h"

// Circular buffers: 
// RX_BUF_SIZE (64): smaller since commands ($HZ,xx*) are short.
// TX_BUF_SIZE (256): larger for the output of $ACC and $ANG messages
#define BUF_SIZE 64
volatile char rx_buffer[BUF_SIZE];
volatile int rx_head = 0;
volatile int rx_tail = 0;

#define TX_BUF_SIZE 256 
volatile char tx_buffer[TX_BUF_SIZE];
volatile int tx_head = 0;
volatile int tx_tail = 0;

#define CS_ACC LATBbits.LATB3 

int freq = 10;      // Dynamic frequency for $ACC transmission
int bandwidth = 15; // Accelerometer filter bandwidth
int ax, ay, az;     // Global variables for shared sensor data

void algorithm() {
    tmr_wait_ms(TIMER2, 7);
}

// UART RX INTERRUPT: non-blocking reception using a circular buffer to prevent data loss even if the CPU is busy with the 7ms algorithm.
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    if (U1STAbits.OERR) U1STAbits.OERR = 0; // Clear overflow error if buffer is full
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

// UART interrupt: handles transmission only when the UART buffer is empty for maintaining the 10ms real-time loop timing.
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    while (!U1STAbits.UTXBF && tx_tail != tx_head) {
        U1TXREG = tx_buffer[tx_tail];
        tx_tail = (tx_tail + 1) % TX_BUF_SIZE;
    }
    IFS0bits.U1TXIF = 0;
}

 // Non-blocking send: it pushes data to the software circular buffer and it manually triggers the TX interrupt if the transmitter is idle.
 
void send_string(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        int next = (tx_head + 1) % TX_BUF_SIZE;
        
        while (next == tx_tail); // Wait only if software buffer is physically full
        tx_buffer[tx_head] = str[i];
        tx_head = next;
    }
    if (IFS0bits.U1TXIF == 0) IFS0bits.U1TXIF = 1;
}

int main(void) {
    // Peripheral Setup
    input_config();
    uart_config();
    spi_config();
    
    // SPI Chip Select Pins
    TRISDbits.TRISD6 = 0; // MAG
    TRISBbits.TRISB4 = 0; // GYRO
    TRISBbits.TRISB3 = 0; // ACC
    
    // Initialize all CS to High (inactive) 
    LATDbits.LATD6 = 1; 
    LATBbits.LATB4 = 1;
    LATBbits.LATB3 = 1;

    TRISGbits.TRISG9 = 0; // Led2 
    
    init_accelerometer();
    set_bandwidth(15); 
    
    // Main loop synchronized at 100Hz (10ms) using Timer 1
    tmr_setup_period(TIMER1, 10); 
    
    int blink_counter = 0;
    int main_counter = 0; // Cycles from 0 to 99 to manage task frequencies
    char msg[20];
    int msg_idx = 0;
    char out_buf[80];

    while (1) {
        algorithm();

        // UART parsing 
        while (rx_head != rx_tail) {
            char c = rx_buffer[rx_tail];
            rx_tail = (rx_tail + 1) % BUF_SIZE;
            
            if (c == '$') 
                msg_idx = 0;
            
            msg[msg_idx++] = c;
            
            if (c == '*') {
                msg[msg_idx] = '\0';
                if (msg[1] == 'H' && msg[2] == 'Z') {
                    int val = atoi(&msg[4]);
                    
                    if (val == 0 || val == 1 || val == 2 || val == 5 || val == 10) 
                        freq = val;
                    
                    else send_string("$ERR,1*\r\n");
                } 
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

        // Data aquisition (50 Hz)
        if (main_counter % 2 == 0) {
            unsigned int lsb, msb;
            CS_ACC = 0;
            
            spi_write(0x02 | 0x80); // Read bit + Start address
            lsb = spi_write(0x00); 
            msb = spi_write(0x00);
            
            ax = (int)((msb << 8) | (lsb & 0xF0)) >> 4; 
            lsb = spi_write(0x00); 
            msb = spi_write(0x00);
            
            ay = (int)((msb << 8) | (lsb & 0xF0)) >> 4;
            lsb = spi_write(0x00); 
            msb = spi_write(0x00);
            
            az = (int)((msb << 8) | (lsb & 0xF0)) >> 4;
            CS_ACC = 1;
        }

        // Accelerometer data transmission 
        if (freq > 0 && (main_counter % (100 / freq) == 0)) {
            sprintf(out_buf, "$ACC,%d,%d,%d*\r\n", ax, ay, az);
            send_string(out_buf);
        }

        // Angle computation and transmission (5 Hz)
        // We add an offset becuse due to the trial we have done, this solution allow us to
        // avoid sending both $ACC and $ANG in the same 10ms cycle, preventing missed deadlines.
        
        if (main_counter % 20 == 5) { 
            double roll = atan2(ay, az) * 57.2958;
            double pitch = atan2(-ax, sqrt((double)ay * ay + (double)az * az)) * 57.2958;
            
            sprintf(out_buf, "$ANG,%.1f,%.1f*\r\n", roll, pitch);
            send_string(out_buf);
        }

        //Led2 blinking 
        //Toggles every 50 cycles (500ms). No busy waiting to
        //allow the rest of the loop to run.
        if (++blink_counter >= 50) {
            LATGbits.LATG9 = !LATGbits.LATG9;
            blink_counter = 0;
        }

        main_counter++;
        if (main_counter >= 100) main_counter = 0;

        //Wait for Timer 1 to complete the 10ms period. If the flag is already 1, 
        //it means the execution exceeded 10ms (Missed Deadline).
 
        if(tmr_wait_period(TIMER1) == 1) {
            send_string("!!! missed deadline !!!\r\n");
        }
    }
}