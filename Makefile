ASM		:= nasm
QEMU	:= qemu-system-i386

BUILD_DIR	:= build
ISO_DIR		:= iso
SRC_DIR		:= vos

SRC			:= $(SRC_DIR)/boot/boot.asm
BIN			:= $(BUILD_DIR)/boot.bin
ISO			:= $(BUILD_DIR)/vos.iso
DISK_IMG	:= $(BUILD_DIR)/vos.img

.PHONY: all iso emulate clean

all: iso

$(BIN): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f bin $< -o $@

$(DISK_IMG): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f bin $< -o $@
	dd if=$@ of=$@ bs=512 count=2880 conv=notrunc

disk: $(DISK_IMG)

$(ISO): $(BIN)
	@mkdir -p $(ISO_DIR)/boot
	@cp $< $(ISO_DIR)/boot/boot.img
	xorriso -as mkisofs \
		-b boot/boot.img \
		-no-emul-boot \
		-o $@ \
		$(ISO_DIR)
	@rm -rf $(ISO_DIR)

iso: $(ISO)


emulate: $(ISO)
	$(QEMU) -cdrom $< -boot d

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)
