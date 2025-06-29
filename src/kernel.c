#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "multiboot.h"
#include "io.h"
#include "memory.h"
#include "helpers.h"
#include "vga.h"
#include "idt.h"
#include "error.h"

void kernel_main(uint32_t multiboot_magic, void* multiboot_info)
{
    init_idt();
    term_set_color(VGA_WHITE, VGA_PURPLE);
    term_clear();
    term_set_cursor_blink(0);

    term_set_fg_color(VGA_YELLOW);
    printf("MOS 0.1.2\n");
    printf("By Justin O'Reilly and Connor Ashcroft\n");

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

    // -- TESTING GROUNDS

    panic("OH SHIT");
    printf("No Errors\n");

    term_set_bg_color(VGA_GREEN);
    printf("success");

    while (1)
    {
        __asm__("hlt");
    }
}
