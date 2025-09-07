PACKAGE_MGR	:= dnf
ASM			:= nasm
GCC			:= gcc
LINKER		:= ld
GRUB_MAKE	:= grub2-mkrescue
QEMU		:= qemu-system-i386

GCC_FLAGS		:= -m32 -fno-stack-protector -fno-builtin -I vos/include
ASM_FLAGS		:= -f elf32
LINKER_FLAGS	:= -m elf_i386
GRUB_MAKE_FLAGS := -o $(ISO)
QEMU_FLAGS		:= -cdrom

BUILD_DIR		:= build
ISO_DIR			:= iso
SRC_DIR			:= vos
SRC_DIR_TMP		:= vos/boot
SRC_FILES		:= $(wildcard $(SRC_DIR_TMP)/*.c)
ASM_SRC_FILES	:= $(wildcard $(SRC_DIR_TMP)/*.asm)
SRC_DIR_TMP		:= vos/kernel
SRC_FILES		+= $(wildcard $(SRC_DIR_TMP)/*.c)

ISO				:= $(BUILD_DIR)/vos.iso

.PHONY: all setup bins iso emulate clean

all: iso

dependencies:
	sudo $(PACKAGE_MGR) install $(ASM) $(GCC) $(LINKER) $(QEMU) glibc-devel.i686 libgcc.i686

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)

setup: clean
	@mkdir -p $(BUILD_DIR)/objects

bins: setup
	$(GCC) $(GCC_FLAGS) -c $(SRC_FILES)
	@mv *.o $(BUILD_DIR)/objects
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/boot/boot.asm -o $(BUILD_DIR)/objects/boot.s.o
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/boot/gdt.asm -o $(BUILD_DIR)/objects/gdt.s.o
	$(LINKER) $(LINKER_FLAGS) -T $(SRC_DIR)/linker.ld build/objects/*.o -o $(BUILD_DIR)/vos.bin

$(ISO): bins
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	cp $(BUILD_DIR)/vos.bin iso/boot/vos.bin
	$(GRUB_MAKE) -o $(BUILD_DIR)/vos.iso iso
	@rm -rf $(ISO_DIR)

iso: $(ISO)

emulate: $(ISO)
	$(QEMU) $(QEMU_FLAGS) $< -boot d
