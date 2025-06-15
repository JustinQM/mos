#include "error.h"

#include <stdint.h>

#include "idt.h"
#include "vga.h"

#define INDEX_TO_VEC  0
#define INDEX_TO_ERR  1

typedef struct
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t vec;        
    uint64_t err_code;   // next on stack
    uint64_t rip, cs, rflags, rsp, ss;
} CPUState;

//what to do before panic fmt is printed
void panic_start()
{
    term_set_color(VGA_WHITE, VGA_RED);
    term_clear();
}

//what to do after panic message is printed
void panic_end()
{
    asm("hlt");
}

void exception_dispatch(uint64_t* stack_frame) {
    // Stack layout (high addr → low):
    //  [ saved registers... ]
    //  [ rdi saved = pointer to here ]
    //  [ vector (1 byte, padded) ]
    //  [ error_code (8 bytes) ]
    //  [ return RIP, CS, RFLAGS, RSP, SS ]

    uint8_t vec = *((uint8_t*)(stack_frame + INDEX_TO_VEC));
    uint64_t err = *(stack_frame + INDEX_TO_ERR);

    CPUState regs = { 0 };
    regs.vec      = vec;
    regs.err_code = err;

    switch (vec) {
        case 2: { panic("Exception %d, Division by Zero, err=%d at %d", vec, err, regs.rip); break;}
        default: panic("Exception %d, err=%d at %d", vec, err, regs.rip);
    }
}
