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
// Does not write to bitmap until successfully allocated
int bitmap_allocate(Bitmap bitmap, size_t count, uint32_t* results);

// Takes the `indices` of the bitmap and frees their respective bits in the `bitmap`
// Returns 0 on success
// Returns 1 if one of the indices were out of range
// Does not account for bits that are already set to 0
// Unlike bitmap_allocate, this WILL write to the bitmap even if it fails
int bitmap_free(Bitmap bitmap, size_t count, uint32_t* indices);

void test_bitmap();
