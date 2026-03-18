#include "es_lib.h"

// Set all the analogical input registers to 0 (digital)
void set_default(){
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
}

// Set up the specified timer with a period in milliseconds
void tmr_setup_period(int timer, int ms){
    if (timer == TIMER1){
        TMR1=0;
        PR1 = (72000000 / 256) * ms / 1000;
        IFS0bits.T1IF = 0;
        T1CONbits.TCKPS = 3;
        T1CONbits.TON = 1;
    }
    
    if(timer == TIMER2){
        TMR2=0;
        PR2 = (72000000 / 256) * ms / 1000;
        IFS0bits.T2IF = 0;
        T2CONbits.TCKPS = 3;
        T2CONbits.TON = 1;
    }
}

// Wait for the timer period to complete and reset the flag
int tmr_wait_period(int timer){
    int ret = 0;
    
    if(timer == TIMER1){
        if(IFS0bits.T1IF == 1){
            ret = 1;
        }else{
            // while to let the flag at 0 while counting 
            while(IFS0bits.T1IF == 0){};
            ret = 0;
        }
    }
    
    if(timer == TIMER2){
        if(IFS0bits.T2IF == 1){
            ret = 1;
        }else{
            // while to let the flag at 0 while counting 
            while(IFS0bits.T2IF == 0){};
            ret = 0;
        }
    }

    // if the timer finishes to count the flag become 1 so we reset it to use the timer again
    IFS0bits.T1IF=0;
    IFS0bits.T2IF=0;
    
    return ret;
}

// Wait for the specified number of milliseconds using the specified timer
void tmr_wait_ms(int timer, int ms){

    if(timer == TIMER1){
        while(ms > 100){
            
            tmr_setup_period(TIMER1, 100);

            while(IFS0bits.T1IF == 0){};

            T1CONbits.TON = 0;
            IFS0bits.T1IF = 0;

            ms = ms - 100;
        }

        if(ms > 0){
            tmr_setup_period(TIMER1, ms);

            while(IFS0bits.T1IF == 0){};

            T1CONbits.TON = 0;
            IFS0bits.T1IF = 0;
        }
    }
    
    if(timer == TIMER2){
        while(ms > 100){
            
            tmr_setup_period(TIMER2, 100);

            while(IFS0bits.T2IF == 0){};

            T2CONbits.TON = 0;
            IFS0bits.T2IF = 0;

            ms = ms - 100;
        }

        if(ms > 0){
            
            tmr_setup_period(TIMER2, ms);

            while(IFS0bits.T2IF == 0){};

            T2CONbits.TON = 0;
            IFS0bits.T2IF = 0;
        }
    } 
}


int count_T1 = 0;

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void){
    IFS0bits.T1IF = 0;

    count_T1++;

    //logic with tmr wait ms
    if (count_T1 == 5){
        LATGbits.LATG6 = !LATGbits.LATG6;
        count_T1 = 0;
    }
}    

void __attribute__((interrupt, auto_psv))_INT2Interrupt(){
    IFS1bits.INT2IF = 0;
    LATGbits.LATG9 = !LATGbits.LATG9;
}