#include "uart_queue.h"

uart_queue_t UART_READ_QUEUE, UART_WRITE_QUEUE;

void uart_queue_init ( )
{
    UART_WRITE_QUEUE.head = UART_WRITE_QUEUE.tail = UART_WRITE_QUEUE.is_full = UART_READ_QUEUE.head = UART_READ_QUEUE.tail = UART_READ_QUEUE.is_full = 0;
}

int uart_enqueue ( uart_queue_t * self, int a )
{
    if ( self->is_full )
        return 0;

    if ( self->head <= self->tail && !self->is_full )
    {
        self->arr[self->tail % UART_QUEUE_LENGTH] = a;
        ( self->tail )++;

        if ( self->tail - self->head + 1 == UART_QUEUE_LENGTH )
            self->is_full = 1;

        return 1;
    }

    return 0;
}
int uart_dequeue ( uart_queue_t * self )
{
    int temp;

    /* the queue is empty */
    if ( self->head == self->tail && !self->is_full )
        return -1;

    temp = self->arr[self->head % UART_QUEUE_LENGTH];
    ( self->head )++;
    self->is_full = 0;

    return temp;
}