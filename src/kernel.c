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

    int* test = malloc(sizeof(int) * 64);
    int* test2 = malloc(sizeof(int) * 64);

    /*if (test2 == NULL)
    {
        printf("oh shit\n");
        return;
    }*/

    printf("Test Address: %d\n", test);

    for (int i = 0; i < 64; i++)
    {
        test[i] = i;
        test2[i] = 64-i;
    }

    for (int i = 0; i < 64; i++)
    {
        printf("%d ", test[i]);
    }

    printf("\n");

    printf("Test 2 Address: %d\n", test2);

    for (int i = 0; i < 64; i++)
    {
        printf("%d ", test2[i]);
    }

    printf("\n");

    free(test);
    free(test2);

    printf("No Errors\n");

    //die
    while (1)
    {
        __asm__("hlt");
    }
}
