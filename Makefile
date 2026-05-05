#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

.SUFFIXES:

BUILD ?= debug

ifeq ($(BUILD), release)
    CARGO_TARGET_DIR := release
    CARGO_FLAGS := --release
else
    CARGO_TARGET_DIR := debug
    CARGO_FLAGS :=
endif

TARGET_DIR := ./target/x86_64-fenrir/$(BUILD)
KERNEL_BIN := $(TARGET_DIR)/kernel
KERNEL_SYMBOLS := $(TARGET_DIR)/kernel_symbols.map
KERNEL_DIRTY  := $(TARGET_DIR)/kernel.stamp

FLANTERM_GIT := ./third_party/flanterm/.git
UACPI_GIT := ./third_party/uacpi/.git
LIMINE_DIR := ./third_party/limine

.PHONY: all run clean $(KERNEL_BIN)

all: fenrir.iso

$(FLANTERM_GIT):
		git clone https://github.com/mintsuki/flanterm.git ./third_party/flanterm --branch=trunk --depth=1

$(UACPI_GIT):
		git clone https://github.com/uACPI/uACPI ./third_party/uacpi --branch=master --depth=1

kernel_build: $(FLANTERM_GIT) $(UACPI_GIT)
	RUSTFLAGS="-C relocation-model=static" cargo build --bin kernel $(CARGO_FLAGS)

$(KERNEL_DIRTY): kernel_build
	@if [ ! -f $(KERNEL_DIRTY) ] || [ $(KERNEL_BIN) -nt $(KERNEL_DIRTY) ]; then \
		nm -Cn $(KERNEL_BIN) | grep -e ' t ' -e ' T ' | cut -d' ' -f1,3- > $(KERNEL_SYMBOLS); \
		touch $(KERNEL_DIRTY); \
	fi

$(LIMINE_DIR):
	mkdir -p ./third_party
	$(eval LIMINE_TAG := $(shell curl -s https://api.github.com/repos/limine-bootloader/limine/releases/latest | grep '"tag_name"' | cut -d'"' -f4))
	curl -L https://github.com/limine-bootloader/limine/releases/download/$(LIMINE_TAG)/limine-binary.tar.gz \
		| tar -xz -C ./third_party --transform 's/^limine-binary/limine/'
	$(MAKE) -C $(LIMINE_DIR)

fenrir.iso: $(KERNEL_DIRTY) $(KERNEL_SYMBOLS) $(LIMINE_DIR)
	mkdir -p ./iso_root/boot
	cp -v $(KERNEL_BIN) ./iso_root/boot/
	cp -v $(TARGET_DIR)/kernel_symbols.map ./iso_root/boot/

	mkdir -p ./iso_root/boot/limine
	cp -v ./limine.conf ./iso_root/boot/limine/
	cp -v $(LIMINE_DIR)/limine-bios.sys ./iso_root/boot/limine/
	cp -v $(LIMINE_DIR)/limine-bios-cd.bin ./iso_root/boot/limine/
	cp -v $(LIMINE_DIR)/limine-uefi-cd.bin ./iso_root/boot/limine/

	mkdir -p ./iso_root/EFI/BOOT
	cp -v $(LIMINE_DIR)/BOOTX64.EFI ./iso_root/EFI/BOOT/
	cp -v $(LIMINE_DIR)/BOOTIA32.EFI ./iso_root/EFI/BOOT/

	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		./iso_root -o fenrir.iso

run: fenrir.iso
	qemu-system-x86_64 \
		-M q35,smm=off \
		-accel tcg \
		-boot d \
		-cdrom fenrir.iso \
		--no-reboot \
		--no-shutdown \
		-m 256M \
		-serial stdio \
		-smp 4

clean:
	cargo clean
	rm -rf ./third_party/ ./iso_root/ fenrir.iso
