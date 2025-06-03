# Toolchain
CC       := gcc 
AS       := nasm
LD       := ld

# Flags for compiling C and assembling ASM
# TODO: Make debug flags not always on
DFLAGS   := -g -O0
RFLAGS   := -O2
CFLAGS   := -m32 -ffreestanding -fno-pic -fno-stack-protector -Wall -Wextra $(DFLAGS)
ASFLAGS  := -f elf

# Linker flags (assumes you have a linker.ld in the project root)
LDFLAGS  := -m elf_i386 -T linker.ld -nostdlib

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

# Run the built kernel in QEMU (i386 multiboot)
.PHONY: qemu
qemu: all
	mkdir -p $(ISO_DIR)/boot/grub
	cp -f iso_grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	cp -f $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot
	grub-mkrescue -o $(BUILD_DIR)/mos.iso $(ISO_DIR)
	qemu-system-i386 -cdrom $(BUILD_DIR)/mos.iso -boot d -display sdl

.PHONY: qemu-debug
qemu-debug:
	mkdir -p $(ISO_DIR)/boot/grub
	cp -f iso_grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	cp -f $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot
	grub-mkrescue -o $(BUILD_DIR)/mos.iso $(ISO_DIR)
	qemu-system-i386 -s -S -cdrom $(BUILD_DIR)/mos.iso -boot d -display sdl &
	gdb $(BUILD_DIR)/kernel.bin -ex "target remote localhost:1234"
