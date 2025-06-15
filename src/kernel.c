#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "multiboot.h"
#include "io.h"
#include "memory.h"
#include "helpers.h"
#include "vga.h"
#include "drive.h"

#include "idt.h"
#include "error.h"

void kernel_main(uint32_t multiboot_magic, void* multiboot_info)
{

    init_idt();
    term_set_color(VGA_WHITE, VGA_PURPLE);
    term_clear();
    term_set_cursor_blink(0);

    term_set_fg_color(VGA_YELLOW);
    printf("MOS 0.1.2? at least 2\n");
    printf("By Justin O'Reilly and Connor Ashcroft\n");
	term_set_fg_color(VGA_WHITE);

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

	printf("identifying drives\n");
	int16_t ident_buffer[256];

	ATADevice device = {prim_base, 0};

	int ident_fail = ata_identify(device, ident_buffer);
	if (ident_fail)
	{
		printf("couldn't identify primary drive 0\n");
		return;
	}

	ATAIdentifyDeviceInfo* ident_info = (ATAIdentifyDeviceInfo*)ident_buffer;
	ata_fix_ident_info(ident_info);
	printf("hey guess what %s\n", &ident_info->model_number);

	if (0)
	{
		char buf[512*3] = "The quick brown fox jumps over the lazy dog";
		memset((void*)(buf+44), 0x69, 512*3-45);
		int result_write = ata_write_sectors(device, 0, 3, buf);
		if (result_write)
		{
			printf("Error writing\n");
		}
		else
		{
			printf("Successful write!\n");
		}
	}
	else
	{
		char buf[512];
		int result_read = ata_read_sectors(device, 1, 1, buf);
		if (result_read)
		{
			printf("Error reading\n");
		}
		else
		{
			printf("Successful read!\n");
			printf("%s<EOF>\n", buf);
		}
	}

}
