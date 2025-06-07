#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "multiboot.h"
#include "io.h"

void kernel_main(uint32_t multiboot_magic, void* multiboot_info)
{
    terminal_setcolor(15, 1); //fg white bg blue
    terminal_clear();

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

    //TODO: Make this a constant
    MMAPInfo mmap_info[256];

    uint32_t mmap_info_length = multiboot_memory_map_load(mb_info->mmap_length, mb_info->mmap_addr, mmap_info);

    if (mmap_info_length <= 0)
    {
        printf("Could not load mmap_info pairs. Got %d pairs\n", mmap_info_length);
        return;
    }

    printf("Loaded %d mmap_info pairs\n", mmap_info_length);

    multiboot_memory_map_print(mmap_info, mmap_info_length);

    //die
    while (1)
    {
        __asm__("hlt");
    }
}
