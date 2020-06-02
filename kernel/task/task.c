#include "task.h"

#include "kernel/memory/mem.h"
#include "kernel/memory/page.h"
#include "kernel/peripherals/time.h"
#include "kernel/peripherals/uart.h"

#include "lib/task.h"
#include "lib/type.h"

#include "schedule.h"
#include "task_queue.h"

thread_info_t * IDLE = NULL;
thread_info_t * THREAD_POOL[NUM_THREADS];

/* only can be used in this file */
thread_info_t * create_thread_info ( void * text_start, uint64_t text_end );

void create_idle_task ( )
{
    pcb_t * idle_pcb = allocate_pcb ( );

    IDLE                = idle_pcb->thread_info;
    IDLE->state         = RUNNING;
    IDLE->func          = idle;
    IDLE->priority      = 1;
    IDLE->const_counter = 0;
    IDLE->counter       = 0;

    IDLE->parent = NULL;
    IDLE->child  = NULL;

    IDLE->kernel_sp = idle_pcb->kernel_stack_ptr;
    IDLE->user_sp   = idle_pcb->user_stack_ptr;

    int i;
    intptr_t addr;
    for ( i = 0; i < USER_STACK_SIZE_PER_TASK / PAGE_SIZE; i++ )
    {
        addr = (intptr_t) user_page_alloc ( idle_pcb->user_pgd_rec );
    }

    ( IDLE->cpu_context ).x[19]        = (uint64_t *) idle;
    ( IDLE->cpu_context ).lr           = (uint64_t *) default_task_start;
    ( IDLE->cpu_context ).user_sp      = (uint64_t *) addr + PAGE_SIZE;
    ( IDLE->cpu_context ).kernel_sp    = (uint64_t *) idle_pcb->kernel_stack_ptr;
    ( IDLE->cpu_context ).user_mode_pc = (uint64_t *) default_task_start;
}

thread_info_t * create_thread_info ( void * text_start, uint64_t text_size )
{
    /* allocate TCB in kernel space */
    pcb_t * new_task            = (pcb_t *) allocate_pcb ( );
    thread_info_t * thread_info = new_task->thread_info;

    /* init a task */
    thread_info->state         = RUNNING;
    thread_info->func          = ( (void ( * ) ( )) text_start );
    thread_info->priority      = 1; /* all task has the same priority */
    thread_info->const_counter = 2;
    thread_info->counter       = 2;

    thread_info->parent = NULL;
    thread_info->child  = NULL;

    thread_info->kernel_sp = new_task->kernel_stack_ptr;

    thread_info->text_start = text_start;
    thread_info->text_size  = text_size;

    unsigned int i;
    intptr_t addr;
    // allocate page for text and data segment
    for ( i = 0; i < text_size / PAGE_SIZE + 1; i++ )
    {
        addr = (intptr_t) user_page_alloc ( new_task->user_pgd_rec );
        memcpy ( (uint64_t *) user_va_to_phy_addr ( new_task->user_pgd_rec, addr ), (uint64_t *) ( ( ( intptr_t ) ( text_start ) ) + PAGE_SIZE * i ), PAGE_SIZE );

        if ( i == 0 )
        {
            ( thread_info->cpu_context ).x[19] = (uint64_t *) addr;
        }
    }

    // allocate page for user stack
    for ( i = 0; i < USER_STACK_SIZE_PER_TASK / PAGE_SIZE; i++ )
    {
        addr = (intptr_t) user_page_alloc ( new_task->user_pgd_rec );
    }

    ( thread_info->cpu_context ).lr           = (uint64_t *) default_task_start;
    ( thread_info->cpu_context ).user_sp      = (uint64_t *) ( addr + PAGE_SIZE );
    ( thread_info->cpu_context ).kernel_sp    = (uint64_t *) ( new_task->kernel_stack_ptr );
    ( thread_info->cpu_context ).user_mode_pc = (uint64_t *) default_task_start;

    sys_printk ( "[TASK]\t\tCreate Task with task_id: %d\n", thread_info->task_id );

    return thread_info;
}

int task_create ( void * text_start, uint64_t text_size )
{
    thread_info_t * new_task = create_thread_info ( text_start, text_size );

    if ( new_task == NULL )
        return -1;

    /* save it into the pool */
    THREAD_POOL[new_task->task_id] = new_task;

    /* put the task into the runqueue */
    task_enqueue ( new_task );

    return new_task->task_id;
}

void set_thread_const_couner ( int pid, int v )
{
    THREAD_POOL[pid]->const_counter = v;
    THREAD_POOL[pid]->counter       = THREAD_POOL[pid]->counter > v ? v : THREAD_POOL[pid]->counter;
}

thread_info_t * get_thread_info ( int pid )
{
    return THREAD_POOL[pid];
}

thread_info_t * sys_duplicate_task ( thread_info_t * current_task )
{
    thread_info_t * new_task = create_thread_info ( current_task->text_start, current_task->text_size );

    if ( new_task == NULL )
        return NULL;

    new_task->priority = current_task->priority;
    new_task->counter  = current_task->counter;

    new_task->parent    = current_task;
    current_task->child = new_task;

    /* copy cpu_context */
    int i;

    /* set the return value as 0 for the child task */
    ( new_task->cpu_context ).x[0] = 0;

    /* copy register, start from 1 */
    for ( i = 1; i < 29; i++ )
        ( new_task->cpu_context ).x[i] = ( new_task->cpu_context ).x[i];
    ( new_task->cpu_context ).fp           = ( new_task->user_sp ) - ( current_task->user_sp - ( ( current_task->cpu_context ).user_sp ) ); /* count the offset */
    ( new_task->cpu_context ).lr           = ( current_task->cpu_context ).lr;                                                              /* return address remain the same */
    ( new_task->cpu_context ).user_sp      = ( new_task->user_sp ) - ( current_task->user_sp - ( ( current_task->cpu_context ).user_sp ) ); /* count the offset */
    ( new_task->cpu_context ).kernel_sp    = new_task->kernel_sp;
    ( new_task->cpu_context ).user_mode_pc = ( current_task->cpu_context ).user_mode_pc; /* program counter remain the same */

    /* copy stack */
    int size = ( uint64_t ) ( (char *) current_task->user_sp - (char *) ( ( current_task->cpu_context ).user_sp ) );
    for ( i = 0; i < size; i++ )
        ( (char *) ( ( new_task->cpu_context ).user_sp ) )[i] = ( (char *) ( ( current_task->cpu_context ).user_sp ) )[i];

    /* save it into the pool */
    THREAD_POOL[new_task->task_id] = new_task;

    /* put the task into the runqueue */
    task_enqueue ( new_task );

    return new_task;
}

void clear_zombie ( )
{
    int i;
    for ( i = 0; i < NUM_THREADS; i++ )
    {
        if ( THREAD_POOL[i]->state == ZOMBIE )
        {
            sys_printk ( "Release Zombie: %d\n", THREAD_POOL[i]->task_id );
            THREAD_POOL[i]->state = DEAD;
            release_pcb ( (pcb_t *) THREAD_POOL[i] );
        }
    }
    sys_wait_msec ( 500000 );
}