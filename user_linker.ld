SECTIONS
{
    . = 0;
    
    _user_start = . ;
        .text.start : {*(.text.start)}
        .text.user : { user/src/*(.text) user/lib/*(.text) }
        .rodata : { user/src/*(.rodata) user/lib/*(.rodata) }
        .data : { user/src/*(.data) user/lib/*(.data) }
        .bss : {
            . = ALIGN(16);
            user/src/*(.bss*)
            user/lib/*(.bss*)
        }
        . = ALIGN(4096);

    _user_end = .;

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }

}