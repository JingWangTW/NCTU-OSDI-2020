#ifndef __SYS_MEM_H
#define __SYS_MEM_H

#include "kernel/peripherals/gpio.h"
#include "kernel/task/task.h"

#include "lib/type.h"

#include "mmu.h"
#include "page.h"

#define KERNEL_STACK_SIZE_PER_TASK ( 1 << 13 ) /* 8K bytes */

#define NUM_THREADS 128

#define TASK_INFO_SIZE    (int) ( sizeof ( task_t ) )
#define PCB_SIZE          16384 /* 16K bytes */
#define KERNEL_SPACE_SIZE ( NUM_THREADS * PCB_SIZE )

#define KERNEL_MEMORY_HIGH ( (volatile char *) MMIO_BASE )
#define KERNEL_MEMORY_LOW  ( (volatile char *) ( (uint64_t) KERNEL_MEMORY_HIGH - KERNEL_SPACE_SIZE ) )
//#define USER_MEMORY_HGIH   KERNEL_MEMORY_LOW
//#define USER_MEMORY_LOW    ( (volatile char *) ) /* variable define in link.ld */

typedef struct
{
    thread_info_t * thread_info;
    uint64_t * kernel_stack_ptr;
    uint64_t * user_stack_ptr;  // fill the blank when task is assigned
    int kernel_space_index;
    // int user_space_index;
    page_t * user_pgd_rec;

} pcb_t;

// defined in mem.S
extern void memzero ( void * addr, uint64_t size );
extern void memcpy ( void * dst, void * src, uint64_t n );

pcb_t * allocate_pcb ( );
void release_pcb ( pcb_t * );
void release_kernel_space ( int index );
void set_ttbr0 ( pcb_t * task );

#endif