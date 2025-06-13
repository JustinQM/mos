#pragma once
#include <stdint.h>
// spec: https://ranthos.com/u/VSLFS-2.0.pdf
#define VSLFS_VERSION 1
#define VSLFS_MAGIC 0x913b21b118ea4594 // this is just a randomly generated 64bit int. any derived filesystems with a different spec should regenerate this
typedef struct
{
	uint64_t magic;			// Magic bytes to identify the filesystem implementation. (see VSLFS_MAGIC)
	uint32_t block_size;	// The size of a block, in bytes (512 for this implementation)
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
	uint64_t block_count;	// Number of blocks the data takes up (including this one)
	uint64_t footer_size;	// The number of bytes of userdata in the last Block it occupies.
	uint8_t userdata[460];	// Bytes of userdata
	uint64_t next_block;	// Block pointer to the next Block with userdata
} FSNode;

typedef struct
{
	uint64_t node;		// Pointer to the node of this file
	char filename[256];	// Filename. include null terminator
} FSDirectory;




