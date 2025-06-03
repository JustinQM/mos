; Code redone from https://wiki.osdev.org/Bare_Bones_with_NASM

; Declare multiboot header
MBALIGN equ 1 << 0 ; align loaded modules on page boundaries
MEMINFO equ 1 << 0 ; provide memory map
MBFLAGS equ MBALIGN | MEMINFO ; this is the Multiboot flag field
MAGIC equ 0x1BADB002 ; magic
CHECKSUM equ -(MAGIC + MBFLAGS) ; checksum of above, should be zero

; declare the multiboot header
section .multiboot
align 4
    dd MAGIC
    dd MBFLAGS
    dd CHECKSUM

; create a 16KiB stack
section .bss ; bss = reserve unitialized data without placing in binary
align 16
stack_bottom:
resb 16384 ; resb = reserve bytes (similar to dd but without placing in binary)
stack_top:

; get bss start and end from the linker
extern __bss_start
extern __bss_end

section .text
global _start:function (_start.end - _start)
_start:
    mov esp, stack_top ; init the stack

    ; this area is for critical processor initalization
    ; before the kernel is entered

    ; zero out bss
    pushad
    xor eax, eax
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    rep stosb ; write AL (0) to [EDI] ECX times
    popad

    ; ending here

    ; declare and call kernel_main
    extern kernel_main
    call kernel_main

    ; if we are out of the kernel, hang forever
    cli ; disable interrupts
.hang: 
    hlt
    jmp .hang
.end: ; _start.end, needed for the global definition of _start for the linker
