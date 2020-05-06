/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef UART_H
#define UART_H

/* PL011 UART registers */
/* https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p.177 */
#define UART_DR   ( (volatile unsigned int *) ( MMIO_BASE + 0x00201000 ) ) /* Data Register */
#define UART_FR   ( (volatile unsigned int *) ( MMIO_BASE + 0x00201018 ) ) /* Flag register */
#define UART_IBRD ( (volatile unsigned int *) ( MMIO_BASE + 0x00201024 ) ) /* Integer Baud Rate Divisor */
#define UART_FBRD ( (volatile unsigned int *) ( MMIO_BASE + 0x00201028 ) ) /* RFractional Baud Rate Divisor */
#define UART_LCRH ( (volatile unsigned int *) ( MMIO_BASE + 0x0020102C ) ) /* Line Control Register */
#define UART_CR   ( (volatile unsigned int *) ( MMIO_BASE + 0x00201030 ) ) /* Control Register */
#define UART_RIS  ( (volatile unsigned int *) ( MMIO_BASE + 0x0020103C ) )
#define UART_IMSC ( (volatile unsigned int *) ( MMIO_BASE + 0x00201038 ) ) /* Interupt FIFO Level Select Register */
#define UART_MIS  ( (volatile unsigned int *) ( MMIO_BASE + 0x00201040 ) )
#define UART_ICR  ( (volatile unsigned int *) ( MMIO_BASE + 0x00201044 ) ) /* Interupt Clear Register */

/* Auxilary mini UART registers */
/* https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p.8 */
#define AUX_ENABLE     ( (volatile unsigned int *) ( MMIO_BASE + 0x00215004 ) ) /* Auxiliary enables */
#define AUX_MU_IO      ( (volatile unsigned int *) ( MMIO_BASE + 0x00215040 ) ) /* Mini Uart I/O Data */
#define AUX_MU_IER     ( (volatile unsigned int *) ( MMIO_BASE + 0x00215044 ) ) /* Mini Uart Interupt Enable */
#define AUX_MU_IIR     ( (volatile unsigned int *) ( MMIO_BASE + 0x00215048 ) ) /* Mini Uart Interupt Identify */
#define AUX_MU_LCR     ( (volatile unsigned int *) ( MMIO_BASE + 0x0021504C ) ) /* Mini Uart Line Control */
#define AUX_MU_MCR     ( (volatile unsigned int *) ( MMIO_BASE + 0x00215050 ) ) /* Mini Uart Modem Control */
#define AUX_MU_LSR     ( (volatile unsigned int *) ( MMIO_BASE + 0x00215054 ) ) /* Mini Uart Line Status */
#define AUX_MU_MSR     ( (volatile unsigned int *) ( MMIO_BASE + 0x00215058 ) ) /* Mini Uart Modem Status */
#define AUX_MU_SCRATCH ( (volatile unsigned int *) ( MMIO_BASE + 0x0021505C ) ) /* Mini Uart Scratch */
#define AUX_MU_CNTL    ( (volatile unsigned int *) ( MMIO_BASE + 0x00215060 ) ) /* Mini Uart Extra Control */
#define AUX_MU_STAT    ( (volatile unsigned int *) ( MMIO_BASE + 0x00215064 ) ) /* Mini Uart Extra Status */
#define AUX_MU_BAUD    ( (volatile unsigned int *) ( MMIO_BASE + 0x00215068 ) ) /* Mini Uart Baudrate */

#define PM_RSTC     ( (volatile unsigned int *) 0x3F10001C )
#define PM_WDOG     ( (volatile unsigned int *) 0x3F100024 )
#define PM_PASSWORD ( 0x5a000000 )

struct uart_buf
{
    char buf[200];
    int head, rear;
} write_buf, read_buf;

enum uart_mode
{
    WRITE = 0,
    READ  = 1
};

typedef enum uart_mode UART_MODE;

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init ( );

/**
 * Send a character
 */
void uart_send ( unsigned int c );

/**
 * Receive a character
 */
char uart_getc ( );

/**
 * Receive a integer
 */
int uart_getint ( );

/**
 * Display a string
 */
void uart_puts ( char * s );

/**
 * Display a string
 */
void uart_printf ( const char * format, ... );

void enable_uart_interrupt ( );
void disable_uart_interrupt ( );
void uart_push ( UART_MODE mode, char c );
int uart_pop ( UART_MODE mode );

#endif