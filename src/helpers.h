#pragma once
#include <stdint.h>
// from https://wiki.osdev.org/Inline_Assembly/Examples#OUTx
static inline void outb(uint16_t port, uint8_t val)
{
	__asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}
static inline void outl(uint16_t port, uint32_t val)
{
	__asm__ volatile ("outl %0,%1" : : "a"(val), "Nd"(port));
}
static inline void outw(uint16_t port, uint16_t val)
{
	__asm__ volatile ("outw %0,%1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	__asm__ volatile ("inb %w1, %b0"
		: "=a"(ret)
		: "Nd"(port)
		: "memory");
	return ret;
}
static inline uint32_t inl(uint16_t port)
{
	uint32_t ret;
	__asm__ volatile ("inl %1,%0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline uint16_t inw(uint16_t port)
{
	uint16_t ret;
	__asm__ volatile ("inw %1,%0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline int flag_get_u32(uint32_t x, unsigned pos)
{
	return (x >> pos) & 1;
}

static inline void flag_set_u32(uint32_t* x, unsigned pos)
{
	*x |= (1u << pos);
}

static inline void flag_clear_u32(uint32_t* x, unsigned pos)
{
	*x &= ~(1 << pos);
}

static inline void flag_toggle_u32(uint32_t* x, unsigned pos)
{
	*x ^= (1 << pos);
}

static inline int flag_get_u8(uint8_t x, unsigned pos)
{
	return (x >> pos) & 1;
}

static inline void flag_set_u8(uint8_t* x, unsigned pos)
{
	*x |= (1u << pos);
}

static inline void flag_clear_u8(uint8_t* x, unsigned pos)
{
	*x &= ~(1 << pos);
}

static inline void flag_toggle_u8(uint8_t* x, unsigned pos)
{
	*x ^= (1 << pos);
}


