#ifndef __SYS_TASK_H
#define __SYS_TASK_H

#include "lib/type.h"

#define USER_STACK_SIZE_PER_TASK ( 1 << 20 ) /* 1M bytes */

typedef enum
{
    RUNNING       = 1,
    DEAD          = 2,
    IDLE_STATE    = 3,
    WAITING_CHILD = 4,
    ZOMBIE        = 5,
} task_state_t;

typedef struct
{
    uint64_t * x[29];  // x0~x28
    uint64_t * fp;     // x29
    uint64_t * lr;     // x30
    uint64_t * user_sp;
    uint64_t * kernel_sp;
    uint64_t * user_mode_pc; /* to save the exec location, while trapped into kernel mode  */

} context_t;

struct thread_info_t
{
    context_t cpu_context;

    uint64_t * user_sp; /* this one will record where the stack start to gorw, and the one in cpu_context will be moved due to the function call */
    uint64_t * kernel_sp;

    struct thread_info_t * parent;
    struct thread_info_t * child;

    uint64_t task_id;
    task_state_t state;
    void * func;

    int64_t const_counter;
    int64_t counter;
    int64_t priority; /* at this time, all task has the same priority */

    uint64_t * text_start; /* where are the text segment */
    uint64_t text_size;    /* the length of text segment */
};

typedef struct thread_info_t thread_info_t;

/* global variable for other file to use */
extern thread_info_t * IDLE;
extern thread_info_t * ZOMBIE_REPEAR;

void create_idle_task ( );
int task_create ( void * text_start, uint64_t text_size );
void set_thread_const_couner ( int pid, int v );
thread_info_t * get_thread_info ( int pid );
thread_info_t * sys_duplicate_task ( thread_info_t * current );
void clear_zombie ( );

#endif