#include "multiboot.h"

uint32_t multiboot_memory_map_load(uint32_t mmap_length, uint32_t mmap_addr, MMAPInfo* buf)
{
    uint32_t count = 0;
    MMAPInfo* entry = (MMAPInfo*)(uintptr_t)mmap_addr;
    uint8_t* end   = (uint8_t*)entry + mmap_length;

    while((uint8_t*)entry < end) 
    {
        buf[count].size = (uint32_t)entry->size;
        buf[count].base_addr = (uint64_t)entry->base_addr;
        buf[count].length = (uint64_t)entry->length;
        buf[count].type = (uint32_t)entry->type;

        /* Advance `entry` by (entry->size + sizeof(entry->size)) bytes */
        uint8_t* next = (uint8_t*)entry + entry->size + sizeof(entry->size);
        entry = (MMAPInfo*)next;
        count++;
    }

    return count;
}

void multiboot_memory_map_print(MMAPInfo* mmap_info, uint32_t length) 
{
    for(uint32_t i = 0; i < length; i++)
    {
        printf("  size      = %d\n", mmap_info[i].size);
        printf("  base_addr = %d\n", mmap_info[i].base_addr);
        printf("  length    = %d\n", mmap_info[i].length);
        printf("  type      = %d\n\n", mmap_info[i].type);
    }
}
