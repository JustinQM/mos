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

	int ident_fail = ata_identify(prim_base, 0, ident_buffer);
	if (ident_fail)
	{
		printf("couldn't identify primary drive 0\n");
		return;
	}

	ATAIdentifyDeviceInfo* ident_info = (ATAIdentifyDeviceInfo*)ident_buffer;
	ata_fix_ident_info(ident_info);
	printf("hey guess what %s\n", &ident_info->model_number);

	char buf[512] = "Hello world!";
	// mark this as "bootable" just because :)
	buf[510] = 0x55;
	buf[511] = 0xAA;

	// THE BELOW CODE WILL OVERWRITE THE MASTER BOOT RECORD OF THE HARD DRIVE CONNECTED
	// WHICH ONE? I DON'T KNOW. ARE YOU FEELING LUCKY?
	uint32_t target_lba = 0;
	int result2 = ata_write_sector(prim_base, 0, target_lba, (uint8_t*)buf);
	if (result2 == 0)
	{
		printf("success!\n");
	}
	else
	{
		term_set_fg_color(VGA_RED);
		printf("FAILURE %d\n", result2);
	}
	

}
