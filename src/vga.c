#include "vga.h"
#include "helpers.h"

#define VEC2INT(x, y) (x+y*VGA_WIDTH)
typedef struct
{
	uint8_t color;
	size_t x;
	size_t y;
} TerminalState;

static TerminalState TERM_STATE = {0x1F, 0, 0};
void term_set_cursor_blink(uint8_t is_blinking)
{
	if (is_blinking)
	{
		outb(0x3D4, 0x0A);
		outb(0x3D5, (inb(0x3D5) & 0xC0) | VGA_SCANLINE_START);

		outb(0x3D4, 0x0B);
		outb(0x3D5, (inb(0x3D5) & 0xE0) | VGA_SCANLINE_END);
	}
	else
	{
		outb(0x3D4, 0x0A);
		outb(0x3D5, 0x20);
	}
}

void term_set_cursor_pos(uint32_t x, uint32_t y)
{
	TERM_STATE.x = x;
	TERM_STATE.y = y;
	uint16_t pos = VEC2INT(x, y);
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));

	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void term_scroll(int32_t lines)
{
	for (size_t dest_row = 0; dest_row < VGA_HEIGHT; dest_row++)
	{
		size_t src_row = dest_row + lines;
		bool is_empty = src_row < 0 || src_row >= VGA_HEIGHT;
		for (size_t col = 0; col < VGA_WIDTH; col++)
		{
			size_t addr = VEC2INT(col, dest_row)*2;
			if (is_empty)
			{
				VGA_ADDR[addr] = 0;
				VGA_ADDR[addr+1] = TERM_STATE.color;
			}
			else
			{
				size_t src_addr = VEC2INT(col, src_row)*2;
				VGA_ADDR[addr] = VGA_ADDR[src_addr];
				VGA_ADDR[addr+1] = VGA_ADDR[src_addr+1];
			}
		}
	}

}


void term_newline()
{
	TERM_STATE.x = 0;
	if (TERM_STATE.y+1 >= VGA_HEIGHT)
	{
		term_scroll(1);
	}
	else
	{
		TERM_STATE.y++;
	}
}

void term_put_char(char c)
{
	if (c == '\n')
	{
		term_newline();
		term_set_cursor_pos(TERM_STATE.x, TERM_STATE.y);
		return;
	}

	if (c == '\r')
	{
		TERM_STATE.x = 0;
		term_set_cursor_pos(TERM_STATE.x, TERM_STATE.y);
		return;
	}

	if ((unsigned)TERM_STATE.x >= VGA_WIDTH)
	{
		term_newline();
	}
	size_t addr = VEC2INT(TERM_STATE.x, TERM_STATE.y) * 2;
	VGA_ADDR[addr] = c;
	VGA_ADDR[addr + 1] = TERM_STATE.color;
	TERM_STATE.x++;

	term_set_cursor_pos(TERM_STATE.x, TERM_STATE.y);
}


void term_write(const char* text)
{
	while (*text != 0)
	{
		char c = *text;
		term_put_char(c);
		text++;
	}

}

void term_set_color(uint8_t fg, uint8_t bg)
{
	TERM_STATE.color = (bg << 4) | (fg & 0x0F);
}

void term_set_fg_color(uint8_t fg)
{
	TERM_STATE.color = (fg & 0xFF) | (TERM_STATE.color & 0xF0);
}

void term_set_bg_color(uint8_t bg)
{
	TERM_STATE.color = (bg << 4) | (TERM_STATE.color & 0x0F);
}

void term_clear()
{
	for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
	{
		VGA_ADDR[i*2] = '\0';
		VGA_ADDR[i * 2 + 1] = TERM_STATE.color;
	}
	term_set_cursor_pos(0, 0);
}


