#ifndef __SYS_UART_QUEUE_H
#define __SYS_UART_QUEUE_H

#define UART_QUEUE_LENGTH 64

typedef struct uart_queue_t
{
    int head;
    int tail;

    char arr[UART_QUEUE_LENGTH];
    int is_full;

} uart_queue_t;

void uart_queue_init ( );
int uart_enqueue ( uart_queue_t * self, int a );
int uart_dequeue ( uart_queue_t * self );

/* global variable for other file to use */
extern uart_queue_t UART_READ_QUEUE, UART_WRITE_QUEUE;

#endif