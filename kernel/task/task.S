.section ".text"

.global run_init_task
.global launch_init

run_init_task:
    # init uart first at el1 before anything start
    bl      uart_init           // defined in uart.c

    # create IDLE   
    bl      create_idle_task    // defined in task.c

    # create ZOMBIE REAPER
    # ldr     x0,     =zombie_reaper
    # bl      task_create

    # set counter of zombie reaper as only 1
    # mov     x1,     #1
    # bl      set_thread_const_couner

    # create main
    ldr     x0,     =_binary_build_user_img_start   // where the acutally main is
    ldr     x1,     =_binary_build_user_img_size
    bl      task_create                             // create "main" task

    # start from init, which will start to scheduling
    bl      launch_main

    # it shall never reach
    ret

# from el1 to el0
launch_main: 

    ldr     x1,     =THREAD_POOL;
    add     x0,     x1,     x0,     lsl #3
    ldr     x0,     [x0]

    
    msr     TPIDR_EL0, x0

    ldr     x19,    [x0, 8 * 19]
    ldr     lr,     [x0, 8 * 30]
    
    msr     ELR_EL1, lr             // when turn form el1 to el0, start from here

    ldr     x9,     [x0, 8 * 31]    //  user sp 
    msr     SP_EL0, x9

    bl      set_ttbr0

    mov lr, #0
    msr  ELR_EL1, lr

    # mov     sp,     #0x70000        // move the el1 sp to the default

    # enable el1 irq
    # bl      irq_el1_enable          // defined in irq.S
    # bl      sys_core_timer_enable   // defined in timer.c

    # and go !!
    eret