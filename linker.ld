ENTRY(_start)

MEMORY
{
    BOOT (r)   : ORIGIN = 0x00000000, LENGTH = 4K
    KERNEL (rx): ORIGIN = 0x00100000, LENGTH = 16M
    STACK (rw) : ORIGIN = 0x01100000, LENGTH = 512M
}

PHDRS
{
    boot PT_LOAD FLAGS (r);
    text PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    .multiboot ALIGN(4):
    {
        KEEP(*(.multiboot)) /* KEEP: Forces the linker to keep this section */
    } > BOOT : boot

    .text : { *(.text*) } > KERNEL : text
    .rodata ALIGN(16): { *(.rodata*) } > KERNEL : text
    .gdt : { *(.gdt*) } > KERNEL : text

    .data ALIGN(16): { *(.data*) } > KERNEL : data


    .bss ALIGN(16):
    {
        __bss_start = .;
        *(.bss*)
        /* chatgpt explanation of *(COMMON) */
        /* In GCC, “COMMON” symbols are uninitialized data defined without an
        explicit section (e.g. int x; at file scope). 
        The linker lumps all COMMON symbols into .bss by default.*/
        *(COMMON)
        __bss_end = .;
    } > KERNEL : data

    .stack ALIGN(16) (NOLOAD):
    {
        __stack_bottom = .;
        . = . + 512M;
        __stack_top = .;
    } > STACK

    /* chatgpt explanation of /DISCARD/ */
    /* This is a special output section name. Literally the keyword DISCARD in 
    forward slashes.

    Anything you put into /DISCARD/ is chopped out of the final 
    binary. Never emitted.

    Here, *(.comment) and *(.eh_frame) are “extra metadata” sections that 
    GCC or LLVM might inject by default 
    (e.g. .comment holds compiler version strings; .eh_frame holds C++ exception tables). 
    In a tiny freestanding kernel, you don’t want any of that.

    By writing *(.comment), you say “for every input object, grab its .comment 
    section” and “place it into /DISCARD/,” which actually means “throw it away.” 
    Same for *(.eh_frame). */
    /DISCARD/ : { *(.comment) *(.eh_frame) }

    _end = .;
}
