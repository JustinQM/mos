; Code redone from https://wiki.osdev.org/Bare_Bones_with_NASM

; Declare multiboot header
MBALIGN equ 1 << 0 ; align loaded modules on page boundaries
MEMINFO equ 1 << 1 ; provide memory lower and upper
MBFLAGS equ MBALIGN | MEMINFO; this is the Multiboot flag field
MAGIC equ 0x1BADB002 ; magic
CHECKSUM equ -(MAGIC + MBFLAGS) ; checksum of above, should be zero

; declare the multiboot header
section .multiboot
align 4
    dd MAGIC
    dd MBFLAGS
    dd CHECKSUM

section .gdt
gdt_start:
    dq 0                               ; null descriptor

    ; code segment: base=0, limit=0xFFFFF, G=1, D=1, executable, read
    dq 0x00CF9A000000FFFF              ; flag bits set for 4 KiB pages

    ; data segment: base=0, limit=0xFFFFF, G=1, D=1, read/write
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1         ; size-1
    dd gdt_start

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
    cmp eax, 0x2BADB002 ; make sure multiboot passed the memory map
    jne .hang ; TODO: make error if multiboot fails to pass mmap

    lgdt  [gdt_descriptor] ; load the GDT

    jmp   0x08:.protected

.protected:
    ; enable sse optimizations
    mov   eax, cr4
    or    eax, (1 << 9)          ; OSFXSR
    or    eax, (1 << 10)         ; OSXMMEXCPT (optional)
    mov   cr4, eax

    ; zero out bss
    xor eax, eax
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    rep stosb ; write AL (0) to [EDI] ECX times

    mov   ax, 0x10        ; flat data selector
    mov   ds, ax
    mov   es, ax
    mov   fs, ax
    mov   gs, ax

    push  ax              ; reload SS safely
    pop   ss

    lea esp, [stack_top] ; init the stack
    and   esp, 0xFFFFFFF0 ; align the stack by clearing the low 4 bits

    ; declare and call kernel_main
    push ebx
    push 0x2BADB002
    extern kernel_main
    call kernel_main

    ; if we are out of the kernel, hang forever
    cli ; disable interrupts
.hang: 
    hlt
    jmp .hang
.end: ; _start.end, needed for the global definition of _start for the linker
