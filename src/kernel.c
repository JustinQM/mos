#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "multiboot.h"
#include "io.h"
#include "memory.h"
#include "helpers.h"
#include "vga.h"
#include "drive.h"

void kernel_main(uint32_t multiboot_magic, void* multiboot_info)
{

    term_set_color(VGA_WHITE, VGA_BLACK);
    term_clear();
    term_set_cursor_blink(0);

    term_set_fg_color(VGA_YELLOW);
    printf("MOS 0.1.2\n");
    printf("By Justin O'Reilly and Connor Ashcroft\n");
	term_set_fg_color(VGA_WHITE);

    char buf[256*30] = "a"; //stack test

    if (multiboot_magic != MULTIBOOT_MAGIC) 
    {
        printf("Invalid Multiboot Magic. Got: %d and expected: %d", multiboot_magic, MULTIBOOT_MAGIC);
        return;
    }

	MultibootInfo* mb_info = (MultibootInfo*)multiboot_info;

	if (memory_init(mb_info))
	{
		term_set_fg_color(VGA_RED);
		printf("Could not initialize memory!\n");
		return;
	}

	printf("Getting first IDE controller...");
	uint16_t prim_base, sec_base;
	int result = find_ide_controllers(&prim_base, &sec_base);

	
	if (result != 0)
	{
		term_set_fg_color(VGA_RED);
		printf("couldn't find one\n");
		return;
	}

	printf("found! %d and %d\n", prim_base, sec_base);
	//const char* buf = "Hello world!";
	// char buf[256] = "Hello world!";
	// ata_write_sector0(prim_base, 0, buf);	
	
	char buf1[256];
	buf1[0] = 'a';
	buf1[1] = '\0';

	char buf2[256] = "a";
	printf("done!");

    term_set_bg_color(VGA_GREEN);
    printf("success");

}
