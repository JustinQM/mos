#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#include "io.h"

#define MULTIBOOT_MAGIC 0x2BADB002 

/* 
 * Multiboot Information Structure
 * (All fields are laid out in order; use `flags` bits to check presence.)
 *
 * Offsets (in bytes):
 *   0   : flags             (uint32_t) 
 *   4   : mem_lower         (uint32_t) if (flags & (1<<0)) != 0
 *   8   : mem_upper         (uint32_t) if (flags & (1<<0)) != 0
 *  12   : boot_device       (uint32_t) if (flags & (1<<1)) != 0
 *  16   : cmdline           (uint32_t) if (flags & (1<<2)) != 0
 *  20   : mods_count        (uint32_t) if (flags & (1<<3)) != 0
 *  24   : mods_addr         (uint32_t) if (flags & (1<<3)) != 0
 *  28-43: syms (a.out or ELF) (use union; 4 fields × 4 bytes) if (flags & ((1<<4)|(1<<5))) != 0
 *  44   : mmap_length       (uint32_t) if (flags & (1<<6)) != 0
 *  48   : mmap_addr         (uint32_t) if (flags & (1<<6)) != 0
 *  52   : drives_length     (uint32_t) if (flags & (1<<7)) != 0
 *  56   : drives_addr       (uint32_t) if (flags & (1<<7)) != 0
 *  60   : config_table      (uint32_t) if (flags & (1<<8)) != 0
 *  64   : boot_loader_name  (uint32_t) if (flags & (1<<9)) != 0
 *  68   : apm_table         (uint32_t) if (flags & (1<<10)) != 0
 *  72   : vbe_control_info  (uint32_t) if (flags & (1<<11)) != 0
 *  76   : vbe_mode_info     (uint32_t) if (flags & (1<<11)) != 0
 *  80   : vbe_mode          (uint16_t) if (flags & (1<<11)) != 0
 *  82   : vbe_interface_seg (uint16_t) if (flags & (1<<11)) != 0
 *  84   : vbe_interface_off (uint16_t) if (flags & (1<<11)) != 0
 *  86   : vbe_interface_len (uint16_t) if (flags & (1<<11)) != 0
 *  88   : framebuffer_addr  (uint64_t) if (flags & (1<<12)) != 0
 *  96   : framebuffer_pitch (uint32_t) if (flags & (1<<12)) != 0
 * 100   : framebuffer_width (uint32_t) if (flags & (1<<12)) != 0
 * 104   : framebuffer_height(uint32_t) if (flags & (1<<12)) != 0
 * 108   : framebuffer_bpp   (uint8_t)  if (flags & (1<<12)) != 0
 * 109   : framebuffer_type  (uint8_t)  if (flags & (1<<12)) != 0
 * 110-115: color_info (6 bytes)   if (flags & (1<<12)) != 0
 */

typedef struct MultibootInfo {
    uint32_t flags;            /* (mandatory) which fields are valid :contentReference[oaicite:1]{index=1} */

    /* If (flags & (1<<0)) */
    uint32_t mem_lower;        /* Amount of lower memory in KB :contentReference[oaicite:2]{index=2} */
    uint32_t mem_upper;        /* Amount of upper memory in KB :contentReference[oaicite:3]{index=3} */

    /* If (flags & (1<<1)) */
    uint32_t boot_device;      /* BIOS disk device used to boot :contentReference[oaicite:4]{index=4} */

    /* If (flags & (1<<2)) */
    uint32_t cmdline;          /* Physical address of kernel command line string :contentReference[oaicite:5]{index=5} */

    /* If (flags & (1<<3)) */
    uint32_t mods_count;       /* Number of boot modules loaded :contentReference[oaicite:6]{index=6} */
    uint32_t mods_addr;        /* Physical address of first module structure :contentReference[oaicite:7]{index=7} */

    /* If (flags & (1<<4)) then a.out symbols are valid; 
     * If (flags & (1<<5)) then ELF section headers are valid :contentReference[oaicite:8]{index=8}
     */
    union {
        struct {                  /* a.out symbol table (if flags[4]) */
            uint32_t tabsize;    /* Size of symbol table in bytes */
            uint32_t strsize;    /* Size of string table in bytes */
            uint32_t addr;       /* Physical address of symbol table */
            uint32_t reserved;   /* Must be 0 */
        } aout_sym;

        struct {                  /* ELF section header table (if flags[5]) */
            uint32_t num;        /* Number of section headers */
            uint32_t size;       /* Size of each section header */
            uint32_t addr;       /* Physical address of section header table */
            uint32_t shndx;      /* Section header string table index */
        } elf_sec;
    } syms; /* occupies offsets 28..43 :contentReference[oaicite:9]{index=9} */

    /* If (flags & (1<<6)) */
    uint32_t mmap_length;      /* Size of memory map buffer :contentReference[oaicite:10]{index=10} */
    uint32_t mmap_addr;        /* Physical address of memory map buffer :contentReference[oaicite:11]{index=11} */

    /* If (flags & (1<<7)) */
    uint32_t drives_length;    /* Size of drive structures buffer :contentReference[oaicite:12]{index=12} */
    uint32_t drives_addr;      /* Physical address of drive structures :contentReference[oaicite:13]{index=13} */

    /* If (flags & (1<<8)) */
    uint32_t config_table;     /* Physical address of ROM configuration table :contentReference[oaicite:14]{index=14} */

    /* If (flags & (1<<9)) */
    uint32_t boot_loader_name; /* Physical address of bootloader name string :contentReference[oaicite:15]{index=15} */

    /* If (flags & (1<<10)) */
    uint32_t apm_table;        /* Physical address of APM table :contentReference[oaicite:16]{index=16} */

    /* If (flags & (1<<11)) */
    uint32_t vbe_control_info; /* Physical address of VBE control info :contentReference[oaicite:17]{index=17} */
    uint32_t vbe_mode_info;    /* Physical address of VBE mode info :contentReference[oaicite:18]{index=18} */
    uint16_t vbe_mode;         /* Current VBE mode (see VBE spec) :contentReference[oaicite:19]{index=19} */
    uint16_t vbe_interface_seg;/* Segment of VBE interface :contentReference[oaicite:20]{index=20} */
    uint16_t vbe_interface_off;/* Offset of VBE interface :contentReference[oaicite:21]{index=21} */
    uint16_t vbe_interface_len;/* Length of VBE interface :contentReference[oaicite:22]{index=22} */

    /* If (flags & (1<<12)) */
    uint64_t framebuffer_addr; /* Physical address of linear frame buffer :contentReference[oaicite:23]{index=23} */
    uint32_t framebuffer_pitch;/* Number of bytes per scanline :contentReference[oaicite:24]{index=24} */
    uint32_t framebuffer_width;/* Framebuffer width in pixels :contentReference[oaicite:25]{index=25} */
    uint32_t framebuffer_height;/* Framebuffer height in pixels :contentReference[oaicite:26]{index=26} */
    uint8_t  framebuffer_bpp;  /* Bits per pixel (e.g., 32) :contentReference[oaicite:27]{index=27} */
    uint8_t  framebuffer_type; /* 0 = indexed, 1 = RGB, 2 = EGA text :contentReference[oaicite:28]{index=28} */

    /* If (flags & (1<<12)):
     *   - If framebuffer_type == 0 (indexed), then palette info is valid.
     *   - If framebuffer_type == 1 (RGB), then RGB field info is valid.
     * This union occupies offsets 110..115 (6 bytes) :contentReference[oaicite:29]{index=29}
     */
    union {
        struct {                 /* Indexed color palette (if type == 0) */
            uint32_t palette_addr; /* Physical address of palette */
            uint16_t palette_num;  /* Number of palette colors */
        } palette;

        struct {                 /* Direct RGB parameters (if type == 1) */
            uint8_t red_field_position;
            uint8_t red_mask_size;
            uint8_t green_field_position;
            uint8_t green_mask_size;
            uint8_t blue_field_position;
            uint8_t blue_mask_size;
        } rgb;
    } color_info;

} MultibootInfo;

typedef struct __attribute__((packed))
{
   uint32_t size;
   uint64_t base_addr;
   uint64_t length;
   uint32_t type; 
} MMAPInfo;


void multiboot_memory_map_print(uint32_t mmap_length, uint32_t mmap_addr_phys) 
{
    MMAPInfo* entry = (MMAPInfo*)(uintptr_t)mmap_addr_phys;
    uint8_t* end   = (uint8_t*)entry + mmap_length;

    while ((uint8_t*)entry < end) 
    {
        /* Now `entry->size` is 20 (for a standard entry), 
           but it could be larger if future fields are added. */
        printf("  size      = %d\n", (uint32_t)entry->size);
        printf("  base_addr = %d\n", (uint64_t)entry->base_addr);
        printf("  length    = %d\n", (uint64_t)entry->length);
        printf("  type      = %d\n\n", (uint32_t)entry->type);

        /* Advance `entry` by (entry->size + sizeof(entry->size)) bytes */
        uint8_t* next = (uint8_t*)entry + entry->size + sizeof(entry->size);
        entry = (MMAPInfo*) next;
    }
}

#endif //MULTIBOOT_H
