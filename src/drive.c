#include "drive.h"
#include "helpers.h"
#include <stdint.h>
#include <stddef.h>
#include "io.h"

// Read a dword from PCI config space
uint32_t pci_conf_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	uint32_t addr =
		(uint32_t)0x80000000        |  // enable bit
		(bus   << 16) |
		(slot  << 11) |
		(func  <<  8) |
		(offset & 0xFC);
	outl(0xCF8, addr);
	return inl(0xCFC);
}

// Find the first IDE controller and return its BARs
int find_ide_controllers(uint16_t *base0, uint16_t *base1)
{
	for (uint16_t bus = 0; bus < 256; bus++)
	{
		for (uint8_t slot = 0; slot < 32; slot++)
		{
			for (uint8_t func = 0; func < 8; func++)
			{
				uint32_t id = pci_conf_read(bus, slot, func, 0x0);
				uint16_t vendor = id & 0xFFFF;
				if (vendor == 0xFFFF) continue;  // no device
				uint8_t prog_if = (pci_conf_read(bus, slot, func, 0x08) >> 8) & 0xFF;
				uint8_t class   = (pci_conf_read(bus, slot, func, 0x08) >> 24) & 0xFF;
				uint8_t subclass= (pci_conf_read(bus, slot, func, 0x08) >> 16) & 0xFF;
				// class==0x01 (Mass Storage), subclass==0x01 (IDE), prog_if bits indicate primary/secondary I/O use
				if (class == 0x01 && subclass == 0x01)
				{
					// Standard BARs: BAR0 = primary base, BAR1 = primary ctrl,
					//               BAR2 = secondary base, BAR3 = secondary ctrl.
					uint32_t bar0 = pci_conf_read(bus, slot, func, 0x10);
					uint32_t bar2 = pci_conf_read(bus, slot, func, 0x18);
					*base0 = (bar0 & 0xFFFFFFFC) ? (bar0 & 0xFFFFFFFC) : 0x1F0;
					*base1 = (bar2 & 0xFFFFFFFC) ? (bar2 & 0xFFFFFFFC) : 0x170;
					return 0;
				}
			}
		}
	}
	return -1;  // not found
}

// Wait until BSY is clear or timeout
int ata_wait(uint16_t io_base)
{
	for (int i = 0; i < 100000; i++)
	{
		uint8_t status = inb(io_base + 7);
		if (!(status & 0x80)) return 0;  // BSY cleared
	}
	return -1;
}

int ata_wait_drq(uint16_t io_base)
{
	for (int i = 0; i < 100000; i++)
	{
		uint8_t status = inb(io_base + 7);
		if (status & 0x08) // DRQ set
			return 0;
		if (status & 0x01) // error
			return -1;
	}
	return -1;
}

// IDENTIFY one drive (drive = 0 for master, 1 for slave)
int ata_identify(uint16_t io_base, uint8_t drive, uint16_t* out_buffer)
{
	// select drive
	outb(io_base + 6, 0xA0 | (drive<<4));
	outb(io_base + 7, 0xEC);
	
	if (ata_wait(io_base)) return -1; // wait for BSY clear

	uint8_t status = inb(io_base + 7);
	if (status == 0)
	{
		// no device
		return -1;
	}
	
	if (ata_wait_drq(io_base)) return -1; // wait for DRQ

	// Drive exists and responded—normally you'd now read 256 words from data port.

	for (int i = 0; i < 256; i++)
	{
		out_buffer[i] = inw(io_base + 0);
	}

	return 0;
}


/**
 * Write exactly one 512-byte sector at LBA=0 on the given ATA channel.
 *
 * @io_base		Base I/O port of the channel (e.g. 0x1F0)
 * @drive		0 = master, 1 = slave
 * @lba			28-bit LBA address (0 to 0x0FFFFFFF)
 * @buf       Pointer to a 512-byte buffer (must be word-aligned)
 * @return		0 on success, -1 on failure
 */
int ata_write_sector(uint16_t io_base, uint8_t drive, uint32_t lba, uint8_t* buf)
{
	if (lba > 0x0FFFFFFF) return 1;
	if (ata_wait(io_base)) return 2;

	// Select the drive and upper 4 bits of the LBA
	uint8_t head = 0xE0 | (drive << 4) | ((lba >>24) & 0x0F);
	outb(io_base + 6, head);

	outb(io_base + 2, 1);					// Sector count = 1
	outb(io_base + 3, lba & 0xFF);			// LBA bits 0-7
	outb(io_base + 4, (lba >> 8) & 0xFF);	// LBA bits 8-15
	outb(io_base + 5, (lba >> 16) & 0xFF);	// LBA bits 16-23

	outb(io_base + 7, 0x30); // WRITE SECTOR command

	if (ata_wait_drq(io_base)) return 3;

	for (int i = 0; i < 256; i++)
	{
		uint16_t word = ((uint16_t*)buf)[i];
		outw(io_base + 0, word);
	}

	// flush the cache
	outb(io_base + 7, 0xE7);
	ata_wait(io_base);

	return 0;
}

void byteswap_string(char* str, size_t size)
{
	if (size%2)
	{
		// must be multiple of 2
		__asm__("hlt"); //lol
	}
	for (size_t i = 0; i < size; i += 2)
	{
		char temp = str[i];
		str[i] = str[i+1];
		str[i+1] = temp;
	}
}
void ata_fix_ident_info(ATAIdentifyDeviceInfo* info)
{
	byteswap_string(info->serial_number, 20);
	byteswap_string(info->firmware_revision, 8);
	byteswap_string(info->model_number, 40);
}

