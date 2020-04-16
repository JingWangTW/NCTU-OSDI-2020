#include "kernel/peripherals/gpio.h"
#include "kernel/peripherals/uart.h"

#include "irq.h"
#include "timer.h"

void irq_controller_el1 ( )
{
    int irq_source = *CORE0_IRQ_SOURCE;
    int deal = 0;

    if ( irq_source & 0b10 )
    {
        uart_printf("Core Timer Interrupt\n");
        core_timer_reload ();

        deal = 1;
    }

    if (irq_source & 0b100000000000 )
    {
        uart_printf("Local Timer Interrupt\n");
        local_timer_reload ();

        deal = 1;
    }

    // GPU IRQ Pending
    if ( *GPU_IRQ_PENDING_BASIC & 0x1 << 25 )
    {
        char r;

        // input from uart
        // read some data 
        if ( *UART_RIS & 0x10 )
        {
            while (*UART_FR & 0x40)
            {
                // receive
                r = (char)(*UART_DR);

                uart_push ( READ, r );
            }
            *UART_ICR = 0b01 << 4;     // Clears the UARTTXINTR interrupt
        } 

        // output to uart
        // write to uart
        if ( *UART_RIS & 0x20 )
        {
            while ( ( r = uart_pop ( WRITE ) ) != -1 )
            {
                while (*UART_FR & 0x20)
                    asm volatile("nop");
                *UART_DR = r;
            }

            *UART_ICR = 0b10 << 4;
        }
        
        // *UART_ICR = 1 << 4;
    }

    if ( !deal )
    {
        uart_printf("Some Interrupt Happened. I don't like it\n");
        uart_printf("%x\n", irq_source);
    }
}

void irq_controller_el2 ( )
{
    int irq_source = *CORE0_IRQ_SOURCE;
    int deal = 0;

    if ( irq_source & 0b10 )
    {
        uart_printf("Core Timer Interrupt\n");
        core_timer_reload ();

        deal = 1;
    }
    
    if (irq_source & 0b100000000000 )
    {
        uart_printf("Local Timer Interrupt\n");
        local_timer_reload ();

        deal = 1;
    }

    if ( !deal )
    {
        uart_printf("Some Interrupt Happened. I don't like it\n");
        uart_printf("%x\n", irq_source);
    }
}

