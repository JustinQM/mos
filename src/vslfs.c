#include "vslfs.h"
#include "string.h"
#include "memory.h"
#include "bitmap.h"

int fs_write_block(FSFileSystem* fs, uint64_t block, uint8_t* data)
{
	return ata_write_sectors(fs->device, fs->sector + block * fs->superblock.block_size, fs->superblock.block_size, data);
}

int fs_read_block(FSFileSystem* fs, uint64_t block, uint8_t* data)
{
	return ata_read_sectors(fs->device, fs->sector + block * fs->superblock.block_size, fs->superblock.block_size, data);
}

void fs_calculate_sizes(size_t blocks_available, uint32_t inodes_per_block, uint32_t bits_per_block, size_t* blocks_inodes, size_t* blocks_data, size_t* blocks_inode_bitmap, size_t* blocks_data_bitmap)
{
	size_t denominator = 2 * inodes_per_block + bits_per_block * (inodes_per_block + 1);
	
	// assign blocks_data at the end. we'll shrink it accordingly depending on how much space we have left.
	// round these up to the nearest int
	*blocks_inodes = (blocks_available * bits_per_block + denominator - 1) / denominator;
	*blocks_inode_bitmap = (blocks_available * inodes_per_block + denominator - 1) / denominator;
	*blocks_data_bitmap = *blocks_inode_bitmap;

	*blocks_data = blocks_available - *blocks_inodes - *blocks_inode_bitmap - *blocks_data_bitmap;

}

FSFileSystem* fs_create_filesystem(ATADevice device, uint64_t sector, uint32_t block_size, uint64_t block_count, const char* volume_name)
{
	FSFileSystem* fs = malloc(sizeof(FSFileSystem));
	if (fs == NULL)
		return NULL;
	memset((void*)fs, 0, sizeof(FSFileSystem));

	fs->superblock.magic = VSLFS_MAGIC;
	fs->superblock.version = VSLFS_VERSION;
	fs->superblock.block_size = block_size;
	fs->superblock.block_count = block_count;
	strncpy(fs->superblock.volume_name, volume_name, 16);

	fs->sector = sector;
	fs->device = device;
	
	/*
		inodes take up much less space than an entire block,
		so we try to squeeze a bunch into just one block.

		worst-case scenario, there is one Data block for every inode
		let's just assume that we'll have one inode for every block
		so we need to find out how many inodes we'd want
		but at the same time, more inodes means less blocks for Data
		also need to factor in the bitmaps
		
		inputs:
		x = blocks available
		c = count of inodes in a block
		cb = count of bits in a block (usually 32768)

		outputs:
		i = inode blocks
		d = data blocks
		bi = blocks for inode bitmap
		bd = blocks for data bitmap

		bi + bd + i + d = x
		or
		2*bd + i + d = x

		i = d/c
			d = c*i
		bd = d/cb
		bi = bd
		
		solve for d:
			
			2*(d/cb) + d/c + d = x
			(2*d)/cb + d/c + d = x
			(2*d*c)/(cb*c) + (d*cb)/(cb*c) + (d*cb*c)/(cb*c) = x
			(2*d*c + d*cb + d*cb*c)/(cb*c) = x
			((2*c + cb + cb*c) * d) / (cb*c) = x
			(2*c + cb + cb*c) * d = x*cb*c
			d = (x*cb*c) / (2*c + cb + cb*c)

		solve for i:
			i = ((x*cb*c) / (2*c + cb + cb*c)) / c
i = (x*cb) / (2*c + cb + cb*c)

		solve for bd:
			bd = ((x * cb * c) / (2*c + cb + cb*c)) / cb
			bd = (x * c) / (2*c + cb + cb*c)

		solve for bi:
			bi = bd

			
		
	*/

	
	uint8_t* data = malloc(512*block_size);
	if (data == NULL)
	{
		free(fs);
		return NULL;
	}

	size_t blocks_inodes, blocks_data, blocks_inode_bitmap, blocks_data_bitmap;
	// subtract the superblock
	fs_calculate_sizes(block_count - 1, block_size*512/VSLFS_INODE_SIZE, block_size*512*8, &blocks_inodes, &blocks_data, &blocks_inode_bitmap, &blocks_data_bitmap);

	printf("blocks_inodes       %d\n", blocks_inodes);
	printf("blocks_data         %d\n", blocks_data);
	printf("blocks_inode_bitmap %d\n", blocks_inode_bitmap);
	printf("blocks_data_bitmap  %d\n", blocks_data_bitmap);

	fs->superblock.inode_size = VSLFS_INODE_SIZE;
	fs->superblock.inode_bitmap_ptr = 1;
	fs->superblock.inode_bitmap_size = blocks_inode_bitmap;
	fs->superblock.data_bitmap_ptr = fs->superblock.inode_bitmap_ptr + blocks_inode_bitmap;
	fs->superblock.data_bitmap_size = blocks_data_bitmap;
	fs->superblock.inode_ptr = fs->superblock.data_bitmap_ptr + blocks_data_bitmap;
	fs->superblock.inode_size = blocks_inodes;
	fs->superblock.data_ptr = fs->superblock.inode_ptr + blocks_inodes;
	fs->superblock.data_size = blocks_data;

	memset((void*)data, 0, 512*block_size);
	memcpy((void*)data, &fs->superblock, sizeof(FSSuperblock));
	int result = fs_write_block(fs, 0, data);
	free(data);
	
	if (result)
	{
		free(fs);
		return NULL;
	}

	return fs;
}


FSFileSystem* fs_attempt_read_filesystem(ATADevice device, uint64_t sector)
{
	FSFileSystem* fs = malloc(sizeof(FSFileSystem));
	if (fs == NULL)
	{
		printf("couldn't alloc fs buffer\n");
		return NULL;
	}
	memset((void*)fs, 0, sizeof(FSFileSystem));

	// we don't know the block size yet, so just read the first sector
	uint8_t* data = malloc(512);
	if (data == NULL)
	{
		free(fs);
		printf("couldn't alloc data buffer\n");
		return NULL;
	}

	memset((void*)data, 0, 512);
	int result = ata_read_sectors(device, sector, 1, data);
	if (result)
	{
		// couldn't read?
		// maybe error at some point. but we dont have error handling so rip.
		// sorry, future us
		free(fs);
		free(data);
		printf("unable to read sector\n");
		return NULL;
	}
	
	memcpy((void*)&fs->superblock, (void*)data, sizeof(FSSuperblock));
	free(data);

	if (fs->superblock.magic != VSLFS_MAGIC || fs->superblock.version > VSLFS_VERSION)
	{
		free(fs);
		printf("invalid magic or version\n");
		printf("magic was %d, expected %d\n", fs->superblock.magic, VSLFS_MAGIC);
		printf("ver was %d, expected %d\n", fs->superblock.version, VSLFS_VERSION);
		return NULL;
	}

	return fs;

}

void test_vslfs()
{
	ATADevice device = get_first_ata_device();
	if (device.io_base == 0)
	{
		printf("failed to get an ata device\n");
		return;
	}

	FSFileSystem* fs = fs_create_filesystem(device, 0, 8, 256 * 1024*1024 / 8 / 512, "Hello!");
	if (fs == NULL)
		printf("Couldn't create filesystem!\n");
	else
		printf("successfully made FS!\n");
}

