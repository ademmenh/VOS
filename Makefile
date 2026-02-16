PACKAGE_MGR	:= dnf
ASM			:= nasm
GCC			:= gcc
LINKER		:= ld
GRUB_MAKE	:= grub2-mkrescue
QEMU		:= qemu-system-i386
GDB			:= gdb

GCC_FLAGS			:= -m32 -fno-stack-protector -fno-builtin
GCC_FLAGS			+= -I vos/include
GCC_DEBUG_FLAGS		:= -g
ASM_FLAGS			:= -f elf32
LINKER_FLAGS		:= -m elf_i386
GRUB_MAKE_FLAGS 	:= -o $(ISO)
QEMU_FLAGS			:= -cdrom
QEMU_DEBUG_FLAGS	:= -s -S

BUILD_DIR			:= build
ISO_DIR				:= iso
SRC_DIR				:= vos
SRC_DIR_TMP			:= vos/kernel
SRC_FILES			+= $(wildcard $(SRC_DIR_TMP)/*.c $(SRC_DIR_TMP)/**/*.c)
ISO					:= $(BUILD_DIR)/vos.iso
DISO				:= $(BUILD_DIR)/vos.iso

.PHONY: all dependencies clean setup bins dbins iso emulate debug demulate asm_bins c_bins c_debug_bins linker

all: iso

dependencies:
	sudo $(PACKAGE_MGR) install $(ASM) $(GCC) $(LINKER) $(QEMU) glibc-devel.i686 libgcc.i686

setup: clean
	@mkdir -p $(BUILD_DIR)/objects

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)

c_bins:
	$(GCC) $(GCC_FLAGS) -c $(SRC_FILES)
	@mv *.o $(BUILD_DIR)/objects

asm_bins:
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/boot/boot.asm -o $(BUILD_DIR)/objects/boot.o
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/kernel/memory/gdt.asm -o $(BUILD_DIR)/objects/gdt.s.o
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/kernel/schedulers/tss.asm -o $(BUILD_DIR)/objects/tss.s.o
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/kernel/utils/io.asm -o $(BUILD_DIR)/objects/io.s.o
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/kernel/routines/idt.asm -o $(BUILD_DIR)/objects/idt.s.o
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/kernel/schedulers/scheduler.asm -o $(BUILD_DIR)/objects/scheduler.s.o
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/kernel/schedulers/task.asm -o $(BUILD_DIR)/objects/task.s.o
	$(ASM) $(ASM_FLAGS) $(SRC_DIR)/kernel/memory/vmm.asm -o $(BUILD_DIR)/objects/vmm.s.o

link:
	$(LINKER) $(LINKER_FLAGS) -T $(SRC_DIR)/linker.ld build/objects/*.o -o $(BUILD_DIR)/vos.bin

bins: c_bins asm_bins link

$(ISO): setup bins
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	cp $(BUILD_DIR)/vos.bin iso/boot/vos.bin
	$(GRUB_MAKE) -o $(ISO) $(ISO_DIR)
	@rm -rf $(ISO_DIR)

iso: $(ISO)

emulate: $(ISO)
	$(QEMU) $(QEMU_FLAGS) $< -boot d


# Debug
c_debug_bins:
	$(GCC) $(GCC_DEBUG_FLAGS) $(GCC_FLAGS) -c $(SRC_FILES)
	@mv *.o $(BUILD_DIR)/objects

debug_bins: c_debug_bins asm_bins link

$(DISO): setup debug_bins
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	cp $(BUILD_DIR)/vos.bin iso/boot/vos.bin
	$(GRUB_MAKE) -o $(ISO) $(ISO_DIR)
	@rm -rf $(ISO_DIR)

demulate: $(DISO)
	$(QEMU) $(QEMU_DEBUG_FLAGS) $(QEMU_FLAGS) $< -boot d

debug:
	gdb $(BUILD_DIR)/vos.bin -ex "target remote localhost:1234"