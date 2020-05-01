#include "kernel/peripherals/uart.h"

double sys_get_current_timestamp ( )
{
    unsigned long int cnt_freq, cnt_tpct;    

    asm volatile(
        "mrs    %0,     CNTFRQ_EL0;"
        "mrs    %1,     CNTPCT_EL0;"
        : "=r" (cnt_freq),  "=r" (cnt_tpct)
        :
    );

    return ((double)cnt_tpct) / cnt_freq;
}

void sys_wait_cycles ( unsigned int n )
{
    if ( n ) 
    {
        while ( n-- )
        {
            asm volatile("nop");
        }
    }
}

void sys_wait_msec ( unsigned int n )
{
    register unsigned long f, t, r;
    
    // get the current counter frequency
    asm volatile ("mrs %0, CNTFRQ_EL0" : "=r"(f));
    
    // read the current counter
    asm volatile ("mrs %0, CNTPCT_EL0" : "=r"(t));
    
    // calculate expire value for counter
    t+=((f/1000)*n)/1000;
    do {
    
        asm volatile ("mrs %0, CNTPCT_EL0" : "=r"(r));
    
    } while( r < t );
}