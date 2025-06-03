ENTRY(_start)

MEMORY
{
    BOOT (r)   : ORIGIN = 0x00000000, LENGTH = 4K
    KERNEL (rx): ORIGIN = 0x00100000, LENGTH = 16M
}

SECTIONS
{
    .multiboot ALIGN(4):
    {
        KEEP(*(.multiboot)) /* KEEP: Forces the linker to keep this section */
    } > BOOT

    .text : /* can't have align here or else it does not jump to the kernel memory region */
    {
        *(.text*) 
    } > KERNEL 

    .rodata ALIGN(4): 
    {
        *(.rodata*)
    }

    .data ALIGN(4):
    {
        *(.data*)
    }

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
    }

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
}
