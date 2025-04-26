ASM := nasm
RM := rm -rf
SRC_DIR := src
BUILD_DIR := build
ASM_FLAGS := -f bin

TARGET := $(BUILD_DIR)/floppy.img
BINARY := $(BUILD_DIR)/main.bin


all: $(TARGET)

$(TARGET): $(BINARY)
	@mkdir -p $(@D)
	@echo "[IMG] Creating floppy image"
	@dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	@dd if=$< of=$@ conv=notrunc status=none
	@echo "[IMG] Created $@"

$(BINARY): $(SRC_DIR)/main.asm
	@mkdir -p $(@D)
	@echo "[ASM] Assembling $<"
	@$(ASM) $(ASM_FLAGS) $< -o $@
	@echo "[ASM] Output: $@"

run: $(TARGET)
	@echo "[QEMU] Starting emulator"
	@qemu-system-i386 -drive file=$(TARGET),format=raw,if=floppy

