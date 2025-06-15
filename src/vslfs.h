#pragma once
#include <stdint.h>
#include <drive.h>

// spec: https://ranthos.com/u/VSLFS-2.0.pdf
#define VSLFS_VERSION 1
#define VSLFS_MAGIC 0x913b21b118ea4594 // this is just a randomly generated 64bit int. any derived filesystems with a different spec should regenerate this

/*
 * Here's the planned spec
 *
 * Volume:			the partition that this filesystem resides in (C:\ in Windows is a volume, / in Linux is a volume)
 * Block:			a contiguous group of disk sectors. typically aim for 4KiB of data in a single block. block size must be static throughout the whole volume
 * Superblock:		the 0th Block of the volume with metadata about the filesystem
 * inode:			A Block that represents a file. it includes the metadata, and pointers to Data Blocks
 * 
 */

typedef struct
{
	uint64_t magic;			// Magic bytes to identify the filesystem implementation. (see VSLFS_MAGIC)
	uint32_t block_size;	// The size of a block, in sectors (sectors are 512 bytes)
	uint64_t block_count;	// Number of blocks on this volume
	char volume_name[16];	// Name of the volume in ASCII

	// addition to the spec
	uint32_t version;			// Version of the spec (see VSLFS_VERSION)
	// multiple bitmap blocks, since a block can only describe no more than 512*8 blocks, or 4096 blocks
	uint64_t data_bitmap_ptr;	// Block address of where the Data bitmap begins
	uint32_t data_bitmap_size;	// Size of the data bitmap, in Blocks
	uint64_t node_bitmap_ptr;	// Block address of where the Node bitmap begins
	uint32_t node_bitmap_size;	// Size of the Node bitmap, in Blocks
} FSSuperblock


typedef struct
{
	FSSuperblock superblock;
	uint64_t sector;
	ATADevice device;
} FSFileSystem;

enum FS_NODE_FLAGS = {
	IS_DIRECTORY =		0x00000001, // Whether the node represents a Directory
	IN_USE = 			0x00000002  // Whether this node is currently in use, preventing any other handles from being created

};

typedef struct
{
	uint32_t flags;			// Bitflags of the Node. see FS_NODE_FLAGS
	uint64_t parent_node;	// Block pointer to the node that contains this. NULL if this is the root node
	uint64_t creation_date;	// UNIX timestamp of when this was created
	uint64_t modified_date; // UNIX timestamp of when this was last modified
	uint64_t block_count;	// Number of blocks the data takes up
	uint64_t footer_size;	// The number of bytes of userdata in the last Block it occupies.
							// Size of the file in bytes can be calculated by:
							// FSSuperblock.block_size * 512 * max(0, block_count-1) + footer_size
} FSNode;

typedef struct
{
	uint64_t node;		// Pointer to the node of this file
	char filename[256];	// Filename. include null terminator
} FSDirectory;


// Create a filesystem at the given sector. Allocates and returns a pointer to FSSuperblock with the info
FSFileSystem* fs_create_filesystem(ATADevice device, uint64_t sector, uint32_t block_size, uint64_t block_count, const char* volume_name);

// Attempts to read a filesystem from the given sector
FSFileSystem* fs_attempt_load_filesystem(ATADevice device, uint64_t sector); // If a filesystem is here, it will load it in memory and return a pointer

int fs_write_block(FSFileSystem* fs, uint64_t block, uint8_t* data);
int fs_read_block(FSFileSystem* fs, uint64_t block, uint8_t data);


