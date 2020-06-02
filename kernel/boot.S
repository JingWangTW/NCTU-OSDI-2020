.section ".text.boot"

#include "kernel/memory/mmu.h"

.global _start

_start:
    # read cpu id, stop slave cores
    mrs     x1,     MPIDR_EL1
    and     x1,     x1,         #3
    
    # cpu id > 0, stop
    cbnz     x1,    loop

   
stack_init:  
    # set stack before our code
    adr     x0,     _start
    mov     sp,     x0

from_el2_to_el1:

    # disable mmu first, toggle the last bit to 0
    mrs     x0,     SCTLR_EL1 
    eor     x0,      x0,        xzr
    msr	    SCTLR_EL1, x0	

    # EL1 use aarch64
    mov     x1,      #(1 << 31) 
    msr     HCR_EL2, x1

    # EL1h (SPSel = 1) with interrupt disabled
    # SPSR_EL2: Holds the saved process state when an exception is taken to EL2.
    # M[3:0]: AArch64 Exception level and selected Stack Pointer. 0b0101 -> EL1h
    # FIAD[9:6]
    mov     x1,     0b001111000101
    msr     SPSR_EL2, x1

    # set up exception handler for el2
    ldr     x1,     =exception_table_el2
    msr     VBAR_EL2, x1
    
    # set up exception handler for el1
    ldr     x1,     =exception_table_el1
    msr     VBAR_EL1, x1

    # load exception return address
    adr     x1,     return_to_el1
    msr     ELR_EL2, x1

    # return to EL1
    eret 

return_to_el1:

    # disable exception for accessing SIMD reg in EL1, it would be used in printf
    # CPACR_EL1: Controls access to trace, SVE, Advanced SIMD and floating-point functionality.
    # FPEN, bits[21:20]: Traps EL0 and EL1 accesses to the SVE, Advanced SIMD, and floating-point registers to EL1, or to EL2 when it is implemented and enabled for the current Security state and HCR_EL2.TGE is 1, from both Execution states.
    mrs     x1,     CPACR_EL1
    orr     x1,     x1,         (3 << 20) // # #0b1100110000000000000000
    msr     CPACR_EL1, x1

    # clear bss
    adr     x1,     __bss_start
    adr     x2,     __bss_end
    sub     x2,     x2,     x1
    bl 	    memzero
    
    # disable interrupt in el1
    mov     x1,     0x3C0
    msr     SPSR_EL1, x1

    # set up page table
    bl      mmu_init

    # set up stack for el1
    mov     x0,     SYS_VA_START
    add     sp,     x0,     LOW_MEMORY

    ## required 5.1: test mmu setting
    ldr     x2, =run_init_task

    # enable mmu, cache remains disabled, toggle the last bit to 1
    mrs     x1,     SCTLR_EL1
    orr     x1 ,    x1,     1
    msr     SCTLR_EL1, x1 
    
    # go to set up init task, and run it
    br      x2

    # this shall not be touched
    b       loop

loop:  
    wfe
    b       loop