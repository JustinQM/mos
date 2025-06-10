#pragma once
#include <stdint.h>

uint32_t pci_conf_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

int find_ide_controllers(uint16_t *base0, uint16_t *base1);

int ata_wait(uint16_t io_base);
int ata_wait_drq(uint16_t io_base);

int ata_write_sector0(uint16_t io_base, uint8_t drive, uint8_t *buf);

// IDENTIFY one drive (drive = 0 for master, 1 for slave)
int ata_identify(uint16_t io_base, uint8_t drive);



