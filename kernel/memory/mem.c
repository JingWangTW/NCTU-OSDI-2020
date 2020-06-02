#include "mem.h"

#include "kernel/task/task.h"

#include "lib/math.h"

#include "page.h"

/* 64 bit to maintain which one is in used */
uint64_t KERNEL_SPACE_USAGE = 0;

pcb_t * allocate_pcb ( )
{
    /* find usable space */
    int kernel_space_index = find_first_0_in_bit ( KERNEL_SPACE_USAGE );
    KERNEL_SPACE_USAGE |= ( 0b1 << kernel_space_index );

    /* allocate memory to store pcb */
    pcb_t * pcb = (pcb_t *) ( ( KERNEL_MEMORY_LOW ) + ( PCB_SIZE * kernel_space_index ) );

    pcb->kernel_stack_ptr = (uint64_t *) ( ( (char *) pcb ) + PCB_SIZE );

    /* allocate user page */
    pcb->user_pgd_rec = user_pgd_alloc ( );

    pcb->kernel_space_index = kernel_space_index;

    pcb->thread_info = (thread_info_t *) ( pcb );

    pcb->thread_info->task_id = kernel_space_index;

    return pcb;
}

void release_pcb ( pcb_t * pcb )
{
    pcb->kernel_stack_ptr = NULL;

    user_pgd_free ( pcb->user_pgd_rec );

    release_kernel_space ( pcb->kernel_space_index );
}

void release_kernel_space ( int index )
{
    if ( index != 0 )
        KERNEL_SPACE_USAGE &= ~( 1UL << index );
}

void set_ttbr0 ( pcb_t * task )
{
    asm volatile( "msr TTBR0_EL1, %0 " : : "r"( task->user_pgd_rec->pgd ) : );
}
