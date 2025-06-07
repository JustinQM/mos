#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "multiboot.h"
#include "io.h"
#include "memory.h"

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

    if (memory_init(mb_info))
    {
        printf("Could not initalize memory\n");
        return;
    }

    //die
    while (1)
    {
        __asm__("hlt");
    }
}
