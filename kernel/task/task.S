.section ".text"

.global run_init_task
.global launch_init

run_init_task:
    # init uart first at el1 before anything start
    bl      uart_init           // defined in uart.c

    # create IDLE   
    bl      create_idle_task    // defined in task.c

    # create ZOMBIE REAPER
    ldr     x0,     =zombie_reaper
    bl      task_create

    # set counter of zombie reaper as only 1
    mov     x1,     #1
    bl      set_thread_const_couner

    # create main
    ldr      x0,     =main      // where the acutally main is
    bl       task_create        // create "main" task

    bl      uart_init

    # start from init, which will start to scheduling
    bl      launch_init  

    # it shall never reach
    ret

# from el1 to el0
launch_init: 

    # set IDLE are going to run
    ldr     x0,     IDLE            // IDLE would be declare in task.c
    
    msr     TPIDR_EL0, x0

    ldr     x19,    [x0, 8 * 19]
    ldr     lr,     [x0, 8 * 30]
    
    msr     ELR_EL1, lr             // when turn form el1 to el0, start from here

    ldr     x9,     [x0, 8 * 31]    //  user sp 
    msr     SP_EL0, x9

    mov     sp,     #0x70000        // move the el1 sp to the default

    # enable el1 irq
    bl      irq_el1_enable          // defined in irq.S
    bl      sys_core_timer_enable   // defined in timer.c

    # and go !!
    eret