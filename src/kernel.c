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
#include "vslfs.h"
#include "bitmap.h"

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
	
	test_bitmap();

	/*
	if (0)
	{
		FSFileSystem* fs = fs_create_filesystem(device, 0, 8, 64, "My Beloved (:");
		if (fs == NULL)
			printf("Couldn't create filesystem!\n");
		else
			printf("successfully made FS!\n");
	}
	else
	{
		FSFileSystem* fs = fs_attempt_read_filesystem(device, 0);
		if (fs == NULL)
		{
			printf("couldn't read filesystem!\n");
		}
		else
		{
			printf("Read filesystem!\n");
			printf("block_size: %d\n", fs->superblock.block_size);
			printf("block_count: %d\n", fs->superblock.block_count);
			printf("volume_name: %s\n", fs->superblock.volume_name);
		}
	}
	*/

}
