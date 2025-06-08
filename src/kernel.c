#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "multiboot.h"
#include "io.h"
#include "helpers.h"
#include "vga.h"

void multiboot_memory_map_print(uint32_t mmap_length, uint32_t mmap_addr_phys) 
{
    MMAPInfo* entry = (MMAPInfo*)(uintptr_t)mmap_addr_phys;
    uint8_t* end   = (uint8_t*)entry + mmap_length;

    while ((uint8_t*)entry < end) 
    {
        /* Now `entry->size` is 20 (for a standard entry), 
           but it could be larger if future fields are added. */
        printf("  size      = %d\n", (uint32_t)entry->size);
        printf("  base_addr = %d\n", (uint64_t)entry->base_addr);
        printf("  length    = %d\n", (uint64_t)entry->length);
        printf("  type      = %d\n\n", (uint32_t)entry->type);

        /* Advance `entry` by (entry->size + sizeof(entry->size)) bytes */
        uint8_t* next = (uint8_t*)entry + entry->size + sizeof(entry->size);
        entry = (MMAPInfo*) next;
    }
}

void kernel_main(uint32_t multiboot_magic, void* multiboot_info)
{
	term_set_color(VGA_WHITE, VGA_PURPLE);
    term_clear();
	term_set_cursor_blink(0);

	term_set_fg_color(VGA_YELLOW);
	printf("MOS 0.1.2\n");
	printf("By Justin O'Reilly and Connor Ashcroft\n");
	term_set_fg_color(VGA_WHITE);

    if (multiboot_magic != MULTIBOOT_MAGIC) 
    {
        printf("Invalid Multiboot Magic. Got: %d and expected: %d", multiboot_magic, MULTIBOOT_MAGIC);
        return;
    }

    MultibootInfo* mb_info = (MultibootInfo*)multiboot_info;

    //memory flags not set
    if (!((mb_info->flags) & (1 << 6)) || !(mb_info->flags & (1 << 0)))
    {
        printf("Mutliboot memory flags not set (1 << 0 && 1 << 6). Got: %d", mb_info->flags);
        return;
    }

    printf("multiboot flag:%d\n", mb_info->flags);
    printf("mmap length:%d\n", mb_info->mmap_length);
    printf("mmap addr:%d\n", mb_info->mmap_addr);

    multiboot_memory_map_print(mb_info->mmap_length, mb_info->mmap_addr);
	
	term_set_bg_color(VGA_GREEN);
	printf("success");
		//die
    while (1)
    {
        __asm__("hlt");
    }
}
