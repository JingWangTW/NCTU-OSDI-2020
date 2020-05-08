#include "uart.h"

#include "lib/ctype.h"
#include "lib/io.h"
#include "lib/stdarg.h"
#include "lib/string.h"
#include "lib/type.h"

#include "gpio.h"
#include "mailbox.h"
#include "uart_queue.h"

/* Set baud rate and characteristics (115200 8N1) and map to GPIO */
void uart_init ( )
{
    register unsigned int reg;

    /*  turn off UART0 */
    *UART_CR = 0;

    mbox_set_clock_to_PL011 ( );

    /* map UART0 to GPIO pins */
    reg = *GPFSEL1;
    reg &= ~( ( 7 << 12 ) | ( 7 << 15 ) ); /* address of gpio 14, 15 */
    reg |= ( 4 << 12 ) | ( 4 << 15 );      /* set to alt0 */

    *GPFSEL1 = reg;
    *GPPUD   = 0; /* enable gpio 14 and 15 */
    reg      = 150;
    while ( reg-- )
    {
        asm volatile( "nop" );
    }

    *GPPUDCLK0 = ( 1 << 14 ) | ( 1 << 15 );
    reg        = 150;
    while ( reg-- )
    {
        asm volatile( "nop" );
    }

    *GPPUDCLK0 = 0; /* flush GPIO setup */

    *UART_ICR  = 0x7FF; /* clear interrupts */
    *UART_IBRD = 2;     /* 115200 baud */
    *UART_FBRD = 0xB;
    *UART_LCRH = 0b11 << 5; /* 8n1 */
    *UART_CR   = 0x301;     /* enable Tx, Rx, disable FIFO */

    *UART_IMSC = 1 << 5; /* enable irq in Tx, Rx */

    uart_queue_init ( );
}

/* Send a character */
void uart_send ( unsigned int c )
{
    char r;
    uart_enqueue ( &UART_WRITE_QUEUE, c );

    if ( c == '\n' )
        uart_enqueue ( &UART_WRITE_QUEUE, '\r' );

    // if rpi is free now
    // send to it
    if ( *UART_FR & 0x80 )
    {
        r        = uart_dequeue ( &UART_WRITE_QUEUE );
        *UART_DR = r;
    }
}

/* Receive a character */
char uart_getc ( )
{
    char r;

    /* wait until something is in the buffer */
    do
    {
        asm volatile( "nop" );

    } while ( *UART_FR & 0x10 );

    /* read it and return */
    r = (char) ( *UART_DR );

    r = r == '\r' ? '\n' : r;

    uart_send ( r );

    /* convert carrige return to newline */
    return r;
}
// int uart_getc ( )
// {
//     int r;
//
//     r = uart_dequeue ( &UART_READ_QUEUE );
//     while ( 1 )
//     {
//         if ( r == -1 )
//             asm volatile( "wfi" );
//         else
//             break;
//     }
//
//     return r == '\r' ? '\n' : r;
// }

/* Display a string */
void uart_puts ( char * s )
{
    while ( *s )
    {
        uart_send ( *s++ );
    }
}

int sys_printk ( const char * format, ... )
{
    const char * current = format;
    char temp_buffer[50];
    int output_char_counts = 0;

    va_list arguments;
    format_t output_format;

    va_type_value value;

    va_start ( arguments, format );

    do
    {
        if ( *current == '%' )
        {
            output_format = parse_format ( &current );

            /* retrive argument */
            switch ( output_format.type )
            {
                case LONG_INT:
                case INT:
                    value.d = va_arg ( arguments, int );
                    itoa ( value.d, temp_buffer, 10 );
                    break;
                case BINARY:
                    value.d = va_arg ( arguments, int );
                    itoa ( value.d, temp_buffer, 2 );
                    break;
                case OCTAL:
                    value.d = va_arg ( arguments, int );
                    itoa ( value.d, temp_buffer, 8 );
                    break;
                case HEXADECIMAL_LOWER:
                    value.d        = va_arg ( arguments, int );
                    temp_buffer[0] = '0';
                    temp_buffer[1] = 'x';
                    itoa ( value.d, temp_buffer + 2, 16 );
                    str_to_lower ( temp_buffer );
                    break;
                case HEXADECIMAL_UPPER:
                case POINTER:
                    value.d        = va_arg ( arguments, int );
                    temp_buffer[0] = '0';
                    temp_buffer[1] = 'X';
                    itoa ( value.d, temp_buffer + 2, 16 );
                    break;
                case DOUBLE:
                    value.f = va_arg ( arguments, double );
                    ftoa ( value.f, temp_buffer, output_format.after_point_length );
                    break;
                case CHAR:
                    value.c        = va_arg ( arguments, int );
                    temp_buffer[0] = value.c;
                    temp_buffer[1] = '\0';
                    break;
                case STRING:
                    value.s = va_arg ( arguments, char * );
                    break;
                default:
                    break;
            }

            /* output signed if needed */
            if ( output_format.signed_flag && output_format.type == INT )
            {
                value.d = va_arg ( arguments, int );

                output_char_counts++;
                uart_send ( value.d > 0 ? '+' : '-' );
            }

            /* padding zero or space */
            if ( output_format.int_length != 0 && output_format.int_length > strlen ( temp_buffer ) && !output_format.is_justify_left )
            {
                while ( output_format.int_length - strlen ( temp_buffer ) > 0 )
                {
                    if ( output_format.is_zero_padding )
                        uart_send ( '0' );
                    else
                        uart_send ( ' ' );

                    output_char_counts++;
                    output_format.int_length--;
                }
            }

            if ( output_format.type == INT && output_format.signed_flag && value.d < 0 )
            {
                output_char_counts += strlen ( temp_buffer + 1 );

                uart_puts ( temp_buffer + 1 );
            }
            else if ( output_format.type == STRING )
            {
                output_char_counts += strlen ( value.s );
                uart_puts ( value.s );
            }
            else
            {
                output_char_counts += strlen ( temp_buffer );
                uart_puts ( temp_buffer );
            }

            /* padding space at the end*/
            if ( output_format.int_length != 0 && output_format.int_length > strlen ( temp_buffer ) && output_format.is_justify_left )
            {
                while ( output_format.int_length - strlen ( temp_buffer ) > 0 )
                {
                    uart_send ( ' ' );

                    output_char_counts++;
                    output_format.int_length--;
                }
            }
        }
        else
        {
            output_char_counts++;
            uart_send ( *current );

            current++;
        }

    } while ( *current != '\0' );

    return 0;
}