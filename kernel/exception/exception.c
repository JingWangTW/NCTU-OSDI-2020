#include "lib/type.h"
#include "kernel/peripherals/time.h"
#include "kernel/peripherals/uart.h"

#include "exception.h"
#include "irq.h"
#include "timer.h"

void exec_controller_el1 ( )
{
    uint64_t elr, esr;
    uint32_t iss;

    asm volatile ( 
        "mrs     %0, ELR_EL1;"
        "mrs     %1, ESR_EL1;"
        : "=r" ( elr ), "=r" ( esr )
        :
    );

    iss = (((uint32_t)esr)) & 0x00FFFFFF;
    
    switch ( iss ) 
    {
        case TEST_SVC:
            print_exec_info ( EL1, elr, esr );
            break;
        case TEST_HVC:
            LAUNCH_SYS_CALL ( HYPERVISORE_CALL_TEST_HVC );
            break;
        case IRQ_EL1_ENABLE:
            irq_el1_enable ();
            break;
        case IRQ_EL1_DISABLE:
            irq_el1_disable ();
            break;
        case CORE_TIMER_ENABLE:
            core_timer_enable ();
            break;
        case CORE_TIMER_DISABLE:
            core_timer_disable ();
            break;
        case PRINT_TIMESTAMP_EL0:
            print_current_timestamp ();
            break;
        case UART_IRQ_ENABLE:
            enable_uart_interrupt();
            break;
        default:
            print_exec_info ( EL1, elr, esr );
            break;
    }
}

void exec_controller_el2 ( )
{
    uint64_t elr, esr;    
    uint32_t iss;

    asm volatile ( 
        "mrs     %0, ELR_EL2;"
        "mrs     %1, ESR_EL2;"
        : "=r" ( elr ), "=r" ( esr )
        :
    );

    iss = (((uint32_t)esr)) & 0x00FFFFFF;
    
    switch ( iss ) 
    {
        case TEST_HVC:
            uart_printf ( "Here, I am in EL2\n" );
            break;
        default:
            print_exec_info ( EL2, elr, esr );
            break;
    }
}

void print_exec_info ( EL el, uint64_t elr, uint64_t esr )
{
    uint32_t ec, iss;

    ec = (((uint32_t)esr)>>26);
    // int il = (((uint32_t)esr)>>25) & 0x1 ;
    iss = (((uint32_t)esr)) & 0x00FFFFFF;

    uart_printf("Current Exception Level: %d\n", el);
    uart_printf("Exception return address: %x\n", elr);
    uart_printf("Exception clss (EC): %x\n", ec);
    uart_printf("Instruction specific syndrome (ISS): %x\n", iss);
}

int get_current_el ()
{
    int el;
    asm volatile (
        "mrs    x0, CurrentEL;"
        "mov    %0, x0;" 
        : "=r" ( el )
    );

    el >>= 2;
    el &= 2;

    return el;
}