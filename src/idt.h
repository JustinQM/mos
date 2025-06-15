#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <stddef.h>

typedef struct __attribute__((packed))
{
    uint16_t offset_lo;    // bits  0–15 of handler address
    uint16_t selector;     // code segment (CS) selector in GDT
    uint8_t  ist;          // bits 0–2 = IST index, rest zero
    uint8_t  type_attr;    // P=1 DPL bits, type=0xE (interrupt gate)
    uint16_t offset_mid;   // bits 16–31 of handler address
    uint32_t offset_hi;    // bits 32–63 of handler address
    uint32_t zero;
} IdtEntry;

#define IDT_ENTRIES 256
#define STUB_SIZE 64

typedef struct __attribute__((packed))
{
    uint16_t limit;
    size_t base;
} IdtRegister;

extern IdtEntry idt[IDT_ENTRIES];
extern IdtRegister idtr;

void init_idt(void);

#endif //IDT_H
