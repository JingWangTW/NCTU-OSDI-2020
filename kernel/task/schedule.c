#include "schedule.h"

#include "kernel/exception/exception.h"
#include "kernel/memory/mem.h"
#include "kernel/peripherals/uart.h"

#include "lib/task.h"

#include "task.h"
#include "task_queue.h"

void context_switch ( thread_info_t * next )
{
    thread_info_t * current_task = get_current_task_el0 ( );

    /* if the next one is the same as the current, do nothing */
    if ( current_task == next )
        return;

    set_next_task_el0 ( next );

    ( next->counter )--;
}

void sys_do_schedule ( )
{
    thread_info_t * current_task = get_current_task_el0 ( );

    /* check if need to resched */
    /* current task still have time to do something */
    /* CURRENT TASK still doing something */
    /* this is not called by IDLE */
    if ( current_task->counter > 0 && current_task->state == RUNNING )
    {
        ( current_task->counter )--;
        return;
    }

    /* enqueue current task to the run queue, if it needed */
    if ( current_task->state == RUNNING && current_task != IDLE )
    {
        current_task->counter = current_task->const_counter;
        task_enqueue ( current_task );
    }

    /* current task spend all its time, switch to the next one */
    thread_info_t * next;
    while ( 1 )
    {
        next = task_dequeue ( );

        if ( next->state == IDLE_STATE )
        {
            task_enqueue ( next );
        }
        else if ( next->state == RUNNING && next->counter < 0 )
        {
            next->counter = next->const_counter;
            task_enqueue ( next );
        }
        else
        {
            break;
        }
    }

    /* switch to the next one */
    context_switch ( next );
}

void sys_do_exec ( void ( *func ) ( ) )
{
    /* get cuurent task */
    thread_info_t * current_thread = get_current_task_el0 ( );

    current_thread->state = RUNNING;
    current_thread->func  = func;

    /* enqueue this task */
    current_thread->counter = 2;

    /* we will do scheuld, right after setup, and it will check it the "current" task still have time or not */
    /* so, it won;t need to be enqueued */
    /* task_enqueue ( current_thread ); */

    ( current_thread->cpu_context ).x[19]        = (uint64_t *) func;
    ( current_thread->cpu_context ).lr           = (uint64_t *) default_task_start;
    ( current_thread->cpu_context ).user_sp      = (uint64_t *) ( current_thread->user_sp ); /* reset stack start point */
    ( current_thread->cpu_context ).kernel_sp    = (uint64_t *) ( current_thread->kernel_sp );
    ( current_thread->cpu_context ).user_mode_pc = (uint64_t *) default_task_start;

    sys_do_schedule ( );
}

void sys_do_exit ( thread_info_t * thread )
{
    /* mark this thread as a dead one */
    thread->state = ZOMBIE;

    // release_user_space ( ( (pcb_t *) thread )->user_space_index );
    sys_printk ( "release user space of task %d\n", thread->task_id );

    sys_do_schedule ( );
}