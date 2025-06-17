#include "bitmap.h"
#include "memory.h"
#include "drive.h"

static size_t get_index_for_bit(size_t bit)
{
	return bit/64;
}

int bitmap_allocate(Bitmap bitmap, size_t count, uint32_t* results)
{
	uint32_t* ptr = bitmap.map;
	uint32_t bit_index = 0; 
	size_t result_index = 0;
	// continue searching each cluster until we run through them all
	while ((size_t)ptr < (size_t)(bitmap.map + bitmap.cluster_count))
	{
		// TODO: should we reserve the first bit in a cluster to mark whether the whole cluster is full? so we don't have to check all 64 bits? could mess up how the indexing works, but maybe there's elegant math that could take care of that. dunno
		uint32_t result_mask = 0; // when finished with each cluster of bits, bitwise OR this mask into the Bitmap so we mark it as occupied
		for (uint8_t i = 0; i < 32; i++)
		{
			uint32_t mask = 1 << i;
			
			// check if bit is not set
			if (!(*ptr & (1 << i)))
			{
				results[result_index++] = bit_index;
				result_mask |= mask;
				if (result_index >= count)
				{
					*ptr |= result_mask;
					return 0;
				}
			}
			bit_index++;
		}
		*ptr |= result_mask;
		ptr++;
	}
	return 1;
}

void test_bitmap()
{
	ATADevice device = get_first_ata_device();
	if (device.io_base == 0)
	{
		printf("failed to get an ata device\n");
		return;
	}


	
	uint32_t bitmap_data[8];
	memset((void*)bitmap_data, 0, sizeof(uint32_t) * 8);
	Bitmap bitmap = {8, bitmap_data};

	bitmap_data[0] = 0b01101100011010111111101111111110;

	uint32_t alloc_results[12];
	int alloc_status = bitmap_allocate(bitmap, 12, alloc_results);

	if (alloc_status)
	{
		printf("failed to allocate bitmap\n");
	}
	else
	{
		printf("successfully allocated bitmap\n");
		for (int i = 0; i < 12; i++)
		{
			printf("%d: %d / ", i, alloc_results[i]);
		}
		printf("\n");

	}
}

