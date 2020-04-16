#include "kernel/peripherals/uart.h"
#include "kernel/exception/exception.h"
#include "shell.h"

int main()
{
    LAUNCH_SYS_CALL ( SYS_CALL_IRQ_EL1_ENABLE );
    LAUNCH_SYS_CALL ( SYS_CALL_UART_IRQ_ENABLE );

    
    // set up serial console
    uart_init();

    // say hello
    uart_printf("Hello World\n");
    
    // start shell
    shell_start();
    
    return 0;
}