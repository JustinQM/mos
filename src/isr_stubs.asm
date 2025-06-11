; isr_stubs.asm
BITS 32 ; 32bit OS
%define STUB_SIZE 64
extern exception_dispatch
GLOBAL isr_stub_base
SECTION .text
isr_stub_base:

%macro ISR_STUB 1
isr_stub_%1:
    push byte %1                ; vector number

; only these vectors auto-push err-code:
%if %1 = 8 | %1 = 10 | %1 = 11 | %1 = 12 | %1 = 13 | %1 = 14 | %1 = 17
    ; nothing
%else
    push dword 0                ; dummy error code
%endif

    mov eax, esp                ; 1st arg = pointer to saved state
    jmp exception_dispatch

    ; pad out to STUB_SIZE
    times STUB_SIZE - ($ - isr_stub_%1) db 0x90
%endmacro

%assign i 0
%rep 32
    ISR_STUB i
    %assign i i+1
%endrep
