# Toolchain
CC       := gcc 
AS       := nasm
LD       := ld

# Flags for compiling C and assembling ASM
# TODO: Make debug flags not always on
DFLAGS   := -g -O0
RFLAGS   := -O2
CFLAGS   := -m64 -ffreestanding -fno-pic -fno-stack-protector -Wall -Wextra $(DFLAGS)
DASFLAGS := -g -F dwarf
ASFLAGS  := -f elf64 $(DASFLAGS)

# Linker flags (assumes you have a linker.ld in the project root)
LDFLAGS  := -m elf_x86_64 -T linker.ld -nostdlib

GRUB_MODULES := part_gpt part_msdos fat \
                normal multiboot multiboot2 \
                efi_gop efi_uga  \
                gfxterm terminal \
                loadenv configfile

# Directories
SRC_DIR   := src
BUILD_DIR := build
ISO_DIR := $(BUILD_DIR)/iso

# Gather all .c and .asm sources under src/
C_SRCS    := $(wildcard $(SRC_DIR)/*.c)
ASM_SRCS  := $(wildcard $(SRC_DIR)/*.asm)

# For each source, produce a corresponding .o in build/
C_OBJS    := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))
ASM_OBJS  := $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SRCS))

# All object files
OBJS      := $(C_OBJS) $(ASM_OBJS)

# Default target
.PHONY: all
all: $(BUILD_DIR) kernel.bin

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/kernel.bin $(OBJS)

# Ensure build directory exists before building .o files
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C sources
# Note the order-only prerequisite '|' ensures build dir is created first
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Remove build directory and all generated files
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: iso_setup
iso_setup:
	mkdir -p $(ISO_DIR)/EFI/BOOT
	grub-mkstandalone --format=x86_64-efi \
	--output=$(ISO_DIR)/EFI/BOOT/BOOTX64.efi \
	--locales="" \
	--fonts="" \
        --modules="$(GRUB_MODULES)" \
	"boot/grub/grub.cfg=$(ISO_DIR)/boot/grub/grub.cfg" \
	"prefix=/EFI/BOOT"

	mkdir -p $(ISO_DIR)/boot/grub

	cp -f iso_grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	cp -f $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot
	grub-mkrescue -o $(BUILD_DIR)/mos.iso $(ISO_DIR) 
	if [ ! -f $(BUILD_DIR)/image.img ]; then \
		qemu-img create $(BUILD_DIR)/image.img 4G; \
	fi

# Run the built kernel in QEMU 
.PHONY: qemu
qemu: all iso_setup
	qemu-system-x86_64 -m 2G \
	-boot d \
	-drive file=$(BUILD_DIR)/image.img,format=raw \
	-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/x64/OVMF_CODE.4m.fd \
	-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/x64/OVMF_VARS.4m.fd \
	-device virtio-vga \
	-display sdl \
        -cdrom $(BUILD_DIR)/mos.iso \

.PHONY: qemu-debug
qemu-debug: all iso_setup
	qemu-system-x84_64 -s -S  -m 2G \
	-boot d \
	-drive file=$(BUILD_DIR)/image.img,format=raw \
	-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/x64/OVMF_CODE.4m.fd \
	-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/x64/OVMF_VARS.4m.fd \
	-device virtio-vga \
	-display sdl \
        -cdrom $(BUILD_DIR)/mos.iso \
	gdb $(BUILD_DIR)/kernel.bin -ex "target remote localhost:1234"
