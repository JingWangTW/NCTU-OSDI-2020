.section ".text"

#include "kernel/exception/sys_call_def.h"

.global default_task_start
.global exec
.global exit
.global get_pid
.global schedule
.global sys_zombie_clearer
.global fork

default_task_start:
    br     x19     // an user mode code 

    # after user mode done, do this routine
    bl      exit 

exec:
    mov     x8,     SYS_DO_EXEC
    svc     #0
    ret

exit:
    mov     x8,     SYS_DO_EXIT
    svc     #0
    ret

get_pid: 
    mov     x8,     SYS_GET_PID
    svc     #0
    ret

schedule:
    mov     x8,     SYS_SCHEDULE
    svc     #0
    ret

sys_zombie_clearer:
    mov     x8,     SYS_CLEAR_ZOMBIE
    svc     #0
    ret

fork:
    mov     x8,     SYS_FORK
    svc     #0
    ret