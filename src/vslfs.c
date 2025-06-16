#include "vslfs.h"
#include "string.h"
#include "memory.h"

int fs_write_block(FSFileSystem* fs, uint64_t block, uint8_t* data)
{
	return ata_write_sectors(fs->device, fs->sector + block * fs->superblock.block_size, fs->superblock.block_size, data);
}

int fs_read_block(FSFileSystem* fs, uint64_t block, uint8_t* data)
{
	return ata_read_sectors(fs->device, fs->sector + block * fs->superblock.block_size, fs->superblock.block_size, data);
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
	
	// TODO: bitmap pointers and sizes
	
	uint8_t* data = malloc(512*block_size);
	if (data == NULL)
	{
		free(fs);
		return NULL;
	}
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


FSFileSystem* fs_attempt_load_filesystem(ATADevice device, uint64_t sector)
{
	FSFileSystem* fs = malloc(sizeof(FSFileSystem));
	if (fs == NULL)
	{
		return NULL;
	}
	memset((void*)fs, 0, sizeof(FSFileSystem));

	// we don't know the block size yet, so just read the first sector
	uint8_t* data = malloc(512);
	if (data == NULL)
	{
		free(fs);
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
		return NULL;
	}
	
	memcpy((void*)&fs->superblock, (void*)data, sizeof(FSSuperblock));

	return fs;

}

