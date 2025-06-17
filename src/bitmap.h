#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct
{
	size_t cluster_count; // number of uint64_t with bitmap info
	uint32_t* map;
} Bitmap;

// Takes a Bitmap and finds the next `count` bits that are unset, and outputs the indices of the bits that were found to be not set
// Returns 0 on success
// Returns 1 if reached end of bitmap when searching (out of space)
int bitmap_allocate(Bitmap bitmap, size_t count, uint32_t* results);

void test_bitmap();
