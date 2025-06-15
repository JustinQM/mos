#include "idt.h"

IdtEntry idt[IDT_ENTRIES];
IdtRegister idtr;

#define INT_GATE 0x8E // P=1, DPL=0, type=14 (32/64-bit interrupt gate)

static void set_idt_entry(int32_t vector, void (*handler)(), uint8_t ist)
{
    uint64_t handler_addr = (size_t)handler;
    idt[vector].offset_lo = handler_addr & 0xFFFF;
    idt[vector].selector = 0x08;
    idt[vector].ist = ist & 0x7;
    idt[vector].type_attr = INT_GATE;
    idt[vector].offset_mid = (handler_addr >> 16) & 0xFFFF;
    idt[vector].offset_hi = (handler_addr >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}

void init_idt(void)
{
    for (int i = 0; i < 32; i++)
    {
        extern uint8_t isr_stub_base[];
        set_idt_entry(i, (void(*)(void))(isr_stub_base + i * STUB_SIZE), 0);
    }

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (size_t)&idt;
    asm volatile ("lidt %0" : : "m"(idtr));
}
