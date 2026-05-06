#include <xc.h>
#include "timer.h"
unsigned int spi_write(unsigned int data){
    
    while (SPI1STATbits.SPITBF == 1);//wait for buffer free
    SPI1BUF = data; //send data
    while(SPI1STATbits.SPIRBF == 0); // wait until data has arrived
    return SPI1BUF;
}


int main(void) {
    
    // We are doing the configuration!
    
    // we disable the analog modality for the D, A ports
    ANSELD = 0;
    ANSELA = 0;
    
    
    //Setup UART
    // we define the R11 register as an input and R0 as output
    TRISDbits.TRISD11 = 1;      // 1 -> input
    TRISDbits.TRISD0 = 0;       // 0 -> output
    
    // now we remap the pins    
    
    // remap for the input which is supposed to be the RD11 pin
    RPINR18bits.U1RXR = 75;         // for input
    
    // remap for the output which is supposed to be the RD0
    RPOR0bits.RP64R = 0x01;         // for output
    
    
    U1MODEbits.BRGH = 0;    // to select the 16 divisor
    U1BRG = 468;            // we load it so we can get 9600
    
    // Power on module and enable transmitter
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
    
    //setup SPI
    
   
    //looked on datasheeet and i think we can just use the one from the slides but not sure...
    TRISAbits.TRISA1 = 1;  // RA1-RPI17  MISO input
    TRISFbits.TRISF12 = 0; // RF12-RP108 SCK output
    TRISFbits.TRISF13 = 0; // RF13-RP109 MOSI output
    
    //                                         zub
    
    RPINR20bits.SDI1R = 0b0010001; // MISO (SDI1) - RPI17 
    RPOR12bits.RP109R = 0b000101;  // MOSI (SDO1) - RF13;
    RPOR11bits.RP108R = 0b000110;  // SCK1; 
    
    // SPI configurations TODO 

    SPI1CON1bits.MSTEN = 1;  // master mode
    SPI1CON1bits.MODE16 = 0; // 8-bit mode
    SPI1STATbits.SPIEN = 1; // enable SPI
    SPI1CON1bits.CKP = 1; // set clock to 1 in idle
    
    //TODO whats whith the rest????
    SPI1CON1bits.PPRE = 3;   // 1:1 primary prescaler
    SPI1CON1bits.SPRE = 3;   // 5:1 secondary prescaler
    SPI1STATbits.SPIROV = 0;
    

    // CS Pins to inactive (high??) we want to use CS3 later
    TRISDbits.TRISD6 = 0;   // CS3/D6 as output
    LATDbits.LATD6 = 1;     // inactive
    
    TRISBbits.TRISB4 = 0;   // CS2 as output
    LATBbits.LATB4 = 1;     // inactive
    
    TRISBbits.TRISB3 = 0;   // CS1 as output
    LATBbits.LATB3 = 1;     // inactive

    
    // variables
    int x;
    unsigned int LSB;
    unsigned int MSB;
    
    int count = 0;
    
    tmr_setup_period(TIMER1, 10);
    

        // sleep mode
        
    LATDbits.LATD6 = 0; // CS3 is used

    spi_write(0x4B & 0x7F); // write address whith MSB=0
    spi_write(0x01); // sleep mode

    LATDbits.LATD6 = 1; // CS3 back to idle
        
    tmr_wait_ms(1,3);  // wait 3 ms
        
        // to active mode
    LATDbits.LATD6 = 0;
    spi_write(0x4C & 0x7F);
    spi_write(0x00); //TODO not sure...what is meant whith opcode 0b??
    LATDbits.LATD6 = 1;
    tmr_wait_ms(1,3);  // wait 3 ms
       
    
    while(1){    
        
        LATDbits.LATD6 = 0;
        spi_write(0x42 | 0x80); //took that register from slide TODO must verify in datasheet
        LSB = spi_write(0x00);
        MSB = spi_write(0x00);
        LATDbits.LATD6 = 1; 
        
        // operations to combine LSB and MSB
        
        LSB = LSB & 0x00F8; 
        MSB = MSB << 8;
        x = (int) (MSB | LSB);
        x = x >> 3;
        
        
                
        // send  x axis to UART
        while(U1STAbits.UTXBF == 1);
        //U1TXREG = x; //TODO not sure if this needs to be char but dont think so??
        char buffer[20];
        sprintf(buffer, "$MAGX=%d*       ", x);
        
        for(int i = 0; buffer[i] != '\0'; i++){
        while(U1STAbits.UTXBF);
        U1TXREG = buffer[i];
        }
         
        tmr_wait_ms(TIMER1, 100);
        
        count++;
             
    }
    
    return 0;
}