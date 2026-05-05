#include <xc.h>
#include <stdlib.h>
#include "config.h"
#include "timer.h"

#define BUF_SIZE 64
volatile char buffer[BUF_SIZE];
volatile int head = 0;
volatile int tail = 0;

void algorithm(){
    tmr_wait_ms(TIMER2,7);
}

// UART1 RX INTERRUPT (Aggiornato per UART1)
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void)
{
    if (U1STAbits.OERR)
        U1STAbits.OERR = 0;

    while (U1STAbits.URXDA)
    {
        char c = U1RXREG;
        int next = (head + 1) % BUF_SIZE;

        if (next != tail)
        {
            buffer[head] = c;
            head = next;
        }
    }
    IFS0bits.U1RXIF = 0; // Flag per UART1 è in IFS0
}

void send_error() {
    char err[] = "$ERR,1*\r\n";
    for (int i = 0; err[i] != '\0'; i++) {
        while (U1STAbits.UTXBF);                // wait for clear buffer 
        U1TXREG = err[i];
    }
}

unsigned int spi_write(unsigned int data){
    while (SPI1STATbits.SPITBF == 1);
    SPI1BUF = data;
    while(SPI1STATbits.SPIRBF == 0);
    return SPI1BUF;
}

int main(void) {
    
    input_config();
    uart_config();
    spi_config();
      
    TRISGbits.TRISG9 = 0;   // LED2 output
    LATGbits.LATG9 = 0;     // variable for blinking led 
    
    // Chip select output 
    TRISDbits.TRISD6 = 0;
    CS_ACC = 1;
    
    accelerometer_config();
    
    tmr_setup_period(TIMER1, 10);
    int blink_counter = 0;
    
    int freq = 10;
    int bandwidth = 15;
    char msg[20];           // buffer for command
    int msg_idx = 0;        // index for reading the command 

    while(1)
    {
        algorithm();
     
        // Reading from UART 
        while(head != tail){
            char c = buffer[tail];
            tail = (tail + 1) % BUF_SIZE;
            
            if(c == '$'){       // start message 
                msg_idx = 0;
            }
            msg[msg_idx++] = c;
            
            if (c == '*'){              // end message
                msg[msg_idx] = '\0';    // end of string 
                
                // Check if it's a Hz command 
                if (msg[1] == 'H' && msg[2] == 'Z') {
                    int val = msg[4] - '0';             // char to number
      
                    if (msg[5] != '*') 
                        val = 10;        // case 2 values number 
                    
                    if (val == 0 || val == 1 || val == 2 || val == 5 || val == 10) {
                        freq = val;
                    } else {
                        send_error(); // Funzione semplice per inviare $ERR,1*
                    }
                }
                
                // Check if it's a BW command
                else if (msg[1] == 'B' && msg[2] == 'W') {
                    // msg è "$BW,xx", il numero parte dalla posizione 4
                    int val = atoi(&msg[4]); 
                    
                    if (val >= 8 && val <= 15) {
                        bandwidth = val;
                        // Qui poi aggiungeremo la funzione per scrivere via SPI 
                        // il nuovo valore nel registro dell'acceleratore
                    } else {
                        send_error();
                    }
                }
                msg_idx = 0; // Reset per il prossimo messaggio
            }
        }
                        
        // blinking led2:
        // to avoid busy-waiting --> cycle counter 
        // the main loop is synchronized at 10ms, 50 iterstions equal 
        // the required 500ms half-period, preserving the remaining 3ms of CPU time
        blink_counter ++;
        if (blink_counter >= 50){
            LATGbits.LATG9 = !LATGbits.LATG9;
            blink_counter = 0;
        }
        
        int ret = tmr_wait_period(TIMER1);
    }

    return 0;
}
