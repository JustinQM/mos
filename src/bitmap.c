#include "bitmap.h"
#include "memory.h"
#include "drive.h"

static size_t get_index_for_bit(size_t bit)
{
	return bit/64;
}

int bitmap_allocate(Bitmap bitmap, size_t count, uint32_t* results)
{
	if (count > bitmap.cluster_count*32)
		return 1; // bruh you trippin
	
	uint32_t* ptr = bitmap.map;
	uint32_t** seen_clusters = malloc(sizeof(uint32_t) * count); // keep track of every cluster we end up going into
	size_t seen_clusters_count = 0;
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
				if (seen_clusters_count == 0 || seen_clusters[seen_clusters_count - 1] != ptr)
				{
					seen_clusters[seen_clusters_count++] = ptr;
				}

				if (result_index >= count)
				{
					// only mark as occupied when we've found enough space
					*ptr |= result_mask;
					for (size_t seen_cluster_index = seen_clusters_count - 2; seen_cluster_index >= 0; seen_cluster_index--)
					{
						*seen_clusters[seen_cluster_index] = 0xFFFFFFFF; // for 32 bit;
						if (seen_cluster_index == 0)
							break; // unsigned moment
					}
					free(seen_clusters);
					return 0;
				}
			}
			bit_index++;
		}
		*ptr |= result_mask; // TODO if we're at this point, doesn't this imply that the cluster is full, and *ptr can just be set to the max uint?
		ptr++;
	}
	free(seen_clusters);
	return 1;
}

int bitmap_free(Bitmap bitmap, size_t count, uint32_t* indices)
{
	for (size_t i = 0; i < count; i++)
	{
		size_t cluster_index = indices[i]/32;
		if (cluster_index >= bitmap.cluster_count)
		{
			return 1; // caller should've seen this. they know the boundaries of the bitmap
			// maybe incorporate some way to undo the writes we did??
		}

		uint32_t mask = 1 << (indices[i]%32);
		bitmap.map[cluster_index] &= ~mask;
	}
	return 0;
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
	bitmap_data[1] = 0b11111111111111011111111111111111;
	bitmap_data[2] = 0b01111111111111111111111111111101;

	printf("initial bitmap: %d, %d, %d, %d\n", bitmap_data[0], bitmap_data[1], bitmap_data[2], bitmap_data[3]);
	uint32_t alloc_results[13];
	int alloc_status = bitmap_allocate(bitmap, 13, alloc_results);

	if (alloc_status)
	{
		printf("failed to allocate bitmap\n");
	}
	else
	{
		printf("successfully allocated bitmap\n");
		for (int i = 0; i < 13; i++)
		{
			printf("%d: %d / ", i, alloc_results[i]);
		}
		printf("\n");
		printf("allocated bitmap: %d, %d, %d, %d\n", bitmap_data[0], bitmap_data[1], bitmap_data[2], bitmap_data[3]);

		printf("freeing bitmap\n");

		if (bitmap_free(bitmap, 13, alloc_results))
		{
			printf("failed to free bitmap\n");
		}
		else
		{
			printf("successfully freed bitmap\n");
			printf("freed bitmap: %d, %d, %d, %d\n", bitmap_data[0], bitmap_data[1], bitmap_data[2], bitmap_data[3]);
		}

	}
}

