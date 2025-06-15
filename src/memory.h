#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include "multiboot.h"

//initalizes memory, returns none zero exit code if an error has occured
uint32_t memory_init(MultibootInfo* mb_info);

void* malloc(size_t size);
void free(void* addr);
void* memset(void* dest, uint8_t value, size_t length);
void* memcpy(void* dest, void* src, size_t size);

void memory_free_list_print(void);

#endif //MEMORY_H
