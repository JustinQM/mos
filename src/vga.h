#pragma once
#include <stdint.h>
#include <stddef.h>
/*
 * VGA text‐mode buffer starts at physical 0xB8000.
 * Each “cell” is two bytes:
 *   byte[0] = ASCII code,
 *   byte[1] = color (high nibble = background, low nibble = foreground).
 */
#define VGA_ADDR ((volatile uint8_t*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define VGA_BLACK	0x0
#define VGA_BLUE	0x1
#define VGA_GREEN	0x2
#define VGA_CYAN	0x3
#define VGA_RED		0x4
#define VGA_MAGENTA	0x5
#define VGA_PURPLE 0x5
#define VGA_ORANGE	0x6
#define VGA_GRAY	0x7
#define VGA_GREY	0x7
#define VGA_DARKGRAY	0x8
#define VGA_DARKGREY	0x8
#define VGA_LIGHTBLUE	0x9
#define VGA_LIGHTGREEN	0xA
#define VGA_LIGHTCYAN	0xB
#define VGA_LIGHTRED	0xC
#define VGA_LIGHTMAGENTA	0xD
#define VGA_PINK	0xD
#define VGA_YELLOW	0xE
#define VGA_WHITE	0xF

#define VGA_SCANLINE_START	0x1
#define VGA_SCANLINE_END	0xE
// Set the foreground and background color for future draw operations
void term_set_color(uint8_t fg, uint8_t bg);

void term_set_fg_color(uint8_t fg);
void term_set_bg_color(uint8_t bg);

void term_put_char(char c);

// Write a null-terminated string to the terminal. Does not wrap text, but auto scrolls.
void term_write(const char* text);

// For each row, copy the row `lines` lines below it and move it there instead.
// `lines` may be negative
// If the source row is outside the range of the terminal, the source row will just be treated as empty.
void term_scroll(int32_t lines);

void term_set_cursor_pos(uint32_t x, uint32_t y);

void term_set_cursor_blink(uint8_t is_blinking);

void term_clear();



