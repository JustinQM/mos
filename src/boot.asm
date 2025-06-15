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

section .bss
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096

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

; get bss start and end from the linker
extern __bss_start
extern __bss_end

; get stack locations from linker
extern __start_bottom
extern __stack_top

section .text

BITS 32
global _start:function (_start.end - _start)
_start:
    cmp eax, 0x2BADB002 ; make sure multiboot passed the memory map
    jne .hang ; TODO: make error if multiboot fails to pass mmap

    call check_cpuid
    call check_long_mode
    call setup_page_tables
    call enable_paging

    lgdt  [gdt_descriptor] ; load the GDT

    mov   ax, 0x10        ; flat data selector
    mov   ds, ax
    mov   es, ax
    mov   fs, ax
    mov   gs, ax
    push  ax              ; reload SS safely
    pop   ss

    ; enable sse optimizations
    mov   eax, cr4
    or    eax, (1 << 9)          ; OSFXSR
    or    eax, (1 << 10)         ; OSXMMEXCPT (optional)
    mov   cr4, eax
    jmp 0x08:.long_mode

BITS 64
.long_mode:
    ; zero out bss
    xor rax, rax
    mov rdi, __bss_start
    mov rcx, __bss_end
    sub rcx, rdi
    rep stosb ; write AL (0) to [EDI] ECX times

    lea rsp, __stack_top ; init the stack
    lea rsp, __stack_top
    and   rsp, 0xFFFFFFFFFFFFFFF0 ; align the stack by clearing the low 4 bits

    ; declare and call kernel_main
    push rbx
    push 0x2BADB002
    extern kernel_main
    call kernel_main

    ; if we are out of the kernel, hang forever
    cli ; disable interrupts

.hang: 
    hlt
    jmp .hang
.end: ; _start.end, needed for the global definition of _start for the linker

BITS 32
; helper funcions
check_cpuid:
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    ; in the FLAGS register. If we can flip it, CPUID is available.

    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax

    ; Copy to ECX as well for comparing later on
    mov ecx, eax

    ; Flip the ID bit
    xor eax, 1 << 21

    ; Copy EAX to FLAGS via the stack
    push eax
    popfd

    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax

    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the
    ; ID bit back if it was ever flipped).
    push ecx
    popfd

    ; Compare EAX and ECX. If they are equal then that means the bit
    ; wasn't flipped, and CPUID isn't supported.
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    mov al, 1
    jmp _start.hang

check_long_mode:
    ; test if extended processor info in available
    mov eax, 0x80000000    ; implicit argument for cpuid
    cpuid                  ; get highest supported argument
    cmp eax, 0x80000001    ; it needs to be at least 0x80000001
    jb .no_long_mode       ; if it's less, the CPU is too old for long mode

    ; use extended info to test if long mode is available
    mov eax, 0x80000001    ; argument for extended processor info
    cpuid                  ; returns various feature bits in ecx and edx
    test edx, 1 << 29      ; test if the LM-bit is set in the D-register
    jz .no_long_mode       ; If it's not set, there is no long mode
    ret
.no_long_mode:
    mov al, 2
    jmp _start.hang

setup_page_tables:
    ; map first P4 entry to P3 table
    mov eax, p3_table
    or eax, 0b11 ; present + writable
    mov [p4_table], eax

    ; map first P3 entry to P2 table
    mov eax, p2_table
    or eax, 0b11 ; present + writable
    mov [p3_table], eax
    ; map each P2 entry to a huge 2MiB page
    mov ecx, 0         ; counter variable

.map_p2_table:
    ; map ecx-th P2 entry to a huge page that starts at address 2MiB*ecx
    mov eax, 0x200000  ; 2MiB
    mul ecx            ; start address of ecx-th page
    or eax, 0b10000011 ; present + writable + huge
    mov [p2_table + ecx * 8], eax ; map ecx-th entry

    inc ecx            ; increase counter
    cmp ecx, 512       ; if counter == 512, the whole P2 table is mapped
    jne .map_p2_table  ; else map the next entry

    ret

enable_paging:
    ; load P4 to cr3 register (cpu uses this to access the P4 table)
    mov eax, p4_table
    mov cr3, eax

    ; enable PAE-flag in cr4 (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; set the long mode bit in the EFER MSR (model specific register)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret
