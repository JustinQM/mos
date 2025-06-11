#pragma once
#include <stdint.h>
/*
 * TODO roadmap:
 * 1. write to mass storage
 * 2. read from mass storage
 * 3. see if there's any pitfalls or unexpected stuff we might encounter on real media?? like bad sectors, failures to read/write, etc
 * 4. implement filesystem based off how we interface with mass storage
 * 5. figure out partitioning or how to get GRUB on mass storage??
 */

typedef struct
{
	uint16_t general_config;
	uint16_t __empty1[9];
	char serial_number[20];
	uint16_t __empty2[3];
	char firmware_revision[8];
	char model_number[40];
	// do the rest later idk
} ATAIdentifyDeviceInfo;

typedef struct
{
	uint16_t io_base;
	uint8_t drive
} ATADevice;

uint32_t pci_conf_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

int find_ide_controllers(uint16_t *base0, uint16_t *base1);

int ata_wait(ATADevice device);
int ata_wait_drq(ATADevice device);

int ata_write_sectors(ATADevice device, uint32_t lba, uint8_t sectors, uint8_t *buf);
int ata_read_sectors(ATADevice device, uint32_t lba, uint8_t sectors, uint8_t *buf);

// IDENTIFY one drive (drive = 0 for master, 1 for slave)
int ata_identify(ATADevice device, uint16_t* out_buffer);

void ata_fix_ident_info(ATAIdentifyDeviceInfo* info);


