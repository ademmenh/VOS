ASM			:= nasm
GCC			:= gcc
LINKER		:= ld
GRUB_MAKE	:= grub2-mkrescue
QEMU		:= qemu-system-i386

BUILD_DIR	:= build
ISO_DIR		:= iso
SRC_DIR		:= vos

SRC			:= $(SRC_DIR)/boot/boot.asm
ISO			:= $(BUILD_DIR)/vos.iso

GCC_FLAGS		:= -m32 -fno-stack-protector -fno-builtin
ASM_FLAGS		:= -f elf32
LINKER_FLAGS	:= -m elf_i386 -T $(SRC_DIR)/linker.ld
GRUB_MAKE_FLAGS := -o $(ISO)
QEMU_FLAGS		:= -cdrom

.PHONY: all setup bins iso emulate clean

all: iso

setup: clean
	@mkdir -p $(BUILD_DIR)/objects

bins: setup
	$(GCC) $(GCC_FLAGS) -c $(SRC_DIR)/kernel/main.c -o $(BUILD_DIR)/objects/main.o
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/boot/boot.asm -o $(BUILD_DIR)/objects/boot.o
	$(LINKER) $(LINKER_FLAGS) build/objects/*.o -o $(BUILD_DIR)/vos.bin

$(ISO): bins
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	cp $(BUILD_DIR)/vos.bin iso/boot/vos.bin
	$(GRUB_MAKE) -o $(BUILD_DIR)/vos.iso iso
	@rm -rf $(ISO_DIR)

iso: $(ISO)

emulate: $(ISO)
	$(QEMU) $(QEMU_FLAGS) $< -boot d

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)
