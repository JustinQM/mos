#pragma once
// from https://wiki.osdev.org/Inline_Assembly/Examples#OUTx
static inline void outb(uint16_t port, uint8_t val)
{
	__asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}
// TODO: add outw and outl


static inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	__asm__ volatile ("inb %w1, %b0"
		: "=a"(ret)
		: "Nd"(port)
		: "memory");
	return ret;
}

