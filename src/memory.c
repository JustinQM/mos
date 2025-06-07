#include "memory.h"

#include "io.h"

#define MMAP_BUFFER_SIZE 256

typedef struct
{
    uint64_t low_memory_addr;
    uint64_t low_memory_length;
    uint64_t memory_addr;
    uint64_t memory_length;
} GlobalMemoryState;

static GlobalMemoryState g_memory_state = { 0 };

uint32_t memory_init(MultibootInfo* mb_info)
{
    printf("multiboot flag:%d\n", mb_info->flags);

    //memory flags not set
    if (!((mb_info->flags) & (1 << 6)) || !(mb_info->flags & (1 << 0)))
    {
        printf("Mutliboot memory flags not set (1 << 0 && 1 << 6). Got: %d", mb_info->flags);
        return 1;
    }

    printf("mmap length:%d\n", mb_info->mmap_length);
    printf("mmap addr:%d\n", mb_info->mmap_addr);

    MMAPInfo mmap_info[MMAP_BUFFER_SIZE];

    uint32_t mmap_info_length = multiboot_memory_map_load(mb_info->mmap_length, mb_info->mmap_addr, mmap_info);

    if (mmap_info_length <= 0)
    {
        printf("Could not load mmap_info pairs. Got %d pairs\n", mmap_info_length);
        return 1;
    }

    printf("Loaded %d mmap_info pairs\n", mmap_info_length);

    //NOTE: This can only find one instance of high memory.
    //Is it possible for their to be multiple instances?
    for (uint32_t i = 0; i < mmap_info_length; i++)
    {
        if (mmap_info[i].type != 1) continue;
        if (mmap_info[i].base_addr == 0)
        {
            g_memory_state.low_memory_addr = mmap_info[i].base_addr;
            g_memory_state.low_memory_length = mmap_info[i].length;
        }
        else //high memory
        {
            g_memory_state.memory_addr = mmap_info[i].base_addr;
            g_memory_state.memory_length = mmap_info[i].length;
        }
    }

    printf("Low Memory Address: %d\n", g_memory_state.low_memory_addr);
    printf("Low Memory Length: %d\n", g_memory_state.low_memory_length);

    printf("Memory Address: %d\n", g_memory_state.memory_addr);
    printf("Memory Length: %d\n", g_memory_state.memory_length);

    return 0;
}
