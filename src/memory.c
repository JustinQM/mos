#include "memory.h"

#include "io.h"

#define MMAP_BUFFER_SIZE 256

typedef struct MemoryHeader
{
    size_t size;
    struct MemoryHeader* previous;
    struct MemoryHeader* next;
    bool free;
} MemoryHeader;

typedef struct
{
    size_t low_memory_addr;
    size_t low_memory_length;

    size_t memory_addr;
    size_t memory_length;

    MemoryHeader* free_list;
} GlobalMemoryState;

static GlobalMemoryState g_memory_state = { 0 };

static size_t align(size_t address, size_t alignment)
{
    return (address + alignment - 1) & ~(alignment - 1);
}

static void split_region(MemoryHeader* region, size_t needed)
{
    if (region->size >= needed + sizeof(MemoryHeader))
    {
        uint8_t* base = (uint8_t*)region;
        MemoryHeader* new_region = (MemoryHeader*)(base + sizeof(MemoryHeader) + needed);
        new_region->size = region->size - needed - sizeof(MemoryHeader);
        new_region->free = true;

        region->size = needed;

        new_region->next = region->next;
        new_region->previous = region;

        if (region->next) region->next->previous = new_region;

        region->next = new_region;
    }
}

static MemoryHeader* find_free(size_t needed)
{
    MemoryHeader* current = g_memory_state.free_list;
    while (current)
    {
        if (current->free && current->size >= needed)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void remove_region_from_free_list(MemoryHeader* region)
{
    if (region->previous) region->previous->next = region->next;
    else g_memory_state.free_list = region->next;
    if (region->next) region->next->previous = region->previous;
    region->previous = NULL;
    region->next = NULL;
}

static void* offset_region_by_header(MemoryHeader* region)
{
    return (void*)((uint8_t*)region + sizeof(MemoryHeader));
}

static MemoryHeader* get_region_header(void* region)
{
    return (MemoryHeader*)((uint8_t*)region - sizeof(MemoryHeader));
}

uint32_t memory_init(MultibootInfo* mb_info)
{
    extern uint8_t _end;

    size_t kernel_end = (size_t)&_end;

    //memory flags not set
    if (!((mb_info->flags) & (1 << 6)) || !(mb_info->flags & (1 << 0)))
    {
        printf("Mutliboot memory flags not set (1 << 0 && 1 << 6). Got: %d", mb_info->flags);
        return 1;
    }

    MMAPInfo mmap_info[MMAP_BUFFER_SIZE];

    uint32_t mmap_info_length = multiboot_memory_map_load(mb_info->mmap_length, mb_info->mmap_addr, mmap_info);

    if (mmap_info_length <= 0)
    {
        printf("Could not load mmap_info pairs. Got %d pairs\n", mmap_info_length);
        return 1;
    }

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

    //init the free_list
    g_memory_state.free_list = (MemoryHeader*)(g_memory_state.memory_addr + kernel_end);
    g_memory_state.free_list->size = g_memory_state.memory_length;
    g_memory_state.free_list->free = true;
    g_memory_state.free_list->previous = NULL;
    g_memory_state.free_list->next = NULL;

    return 0;
}

void* malloc(size_t size)
{
    size = align(size, sizeof(void*));
    MemoryHeader* region = find_free(size);
    if (region == NULL) return NULL; //buy more ram LOL

    split_region(region, size);
    region->free = false;
    remove_region_from_free_list(region);

    return offset_region_by_header(region);
}

void free(void* addr)
{
    MemoryHeader* region = get_region_header(addr);
    MemoryHeader* free_list_entry = g_memory_state.free_list;

    while ((uintptr_t)free_list_entry < (uintptr_t)region)
    {
        if (free_list_entry->next == NULL) break;
        free_list_entry = free_list_entry->next;
    }

    region->free = true;

    //become the new head
    if (free_list_entry->previous == NULL)
    {
        region->next = free_list_entry;
        free_list_entry->previous = region;
        g_memory_state.free_list = region;
    }
    //become the new tail
    else if ((uintptr_t)region > (uintptr_t)free_list_entry)
    {
        region->previous = free_list_entry;
        free_list_entry->next = region;
    }
    else
    {
        region->next = free_list_entry;
        region->previous = free_list_entry->previous;
        free_list_entry->previous = region;
        region->previous->next = region;
    }
}

void memory_free_list_print(void)
{
    MemoryHeader* free_list_entry = g_memory_state.free_list;
    int i = 0;
    printf("Free List\n");
    while (free_list_entry)
    {
        printf("Entry %d\n",i);
        printf("Address: %d\n",free_list_entry);
        printf("Size: %d\n",free_list_entry->size);
        printf("Previous: %d\n",free_list_entry->previous);
        printf("Next: %d\n",free_list_entry->next);
        printf("\n");

        i++;
        free_list_entry = free_list_entry->next;
    }
}
