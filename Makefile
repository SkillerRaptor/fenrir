#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

.SUFFIXES:

BUILD ?= debug

ifeq ($(BUILD), release)
    CARGO_TARGET_DIR := release
else
    CARGO_TARGET_DIR := debug
endif

.PHONY: all
all: fenrir.iso

.PHONY: kernel
kernel: flanterm uacpi
	$(MAKE) -C kernel BUILD=$(BUILD)

flanterm:
ifeq ($(wildcard ./kernel/bindings/flanterm/flanterm),)
		git clone https://github.com/mintsuki/flanterm.git ./kernel/bindings/flanterm/flanterm --branch=trunk --depth=1
endif

uacpi:
ifeq ($(wildcard ./kernel/bindings/uacpi/uacpi),)
		git clone https://github.com/uACPI/uACPI ./kernel/bindings/uacpi/uacpi --branch=master --depth=1
endif

limine:
	mkdir -p ./third_party
ifeq ($(wildcard ./third_party/limine),)
	$(eval LIMINE_TAG := $(shell curl -s https://api.github.com/repos/limine-bootloader/limine/releases/latest | grep '"tag_name"' | cut -d'"' -f4))
	curl -L https://github.com/limine-bootloader/limine/releases/download/$(LIMINE_TAG)/limine-binary.tar.gz \
		| tar -xz -C ./third_party --transform 's/^limine-binary/limine/'
	$(MAKE) -C ./third_party/limine
endif

fenrir.iso: limine kernel
	mkdir -p ./iso_root/boot
	cp -v ./kernel/target/x86_64-fenrir/$(CARGO_TARGET_DIR)/kernel ./iso_root/boot/
	cp -v ./kernel/target/x86_64-fenrir/$(CARGO_TARGET_DIR)/kernel_symbols.map ./iso_root/boot/

	mkdir -p ./iso_root/boot/limine
	cp -v ./limine.conf ./iso_root/boot/limine/
	cp -v ./third_party/limine/limine-bios.sys ./iso_root/boot/limine/
	cp -v ./third_party/limine/limine-bios-cd.bin ./iso_root/boot/limine/
	cp -v ./third_party/limine/limine-uefi-cd.bin ./iso_root/boot/limine/

	mkdir -p ./iso_root/EFI/BOOT
	cp -v ./third_party/limine/BOOTX64.EFI ./iso_root/EFI/BOOT/
	cp -v ./third_party/limine/BOOTIA32.EFI ./iso_root/EFI/BOOT/

	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		./iso_root -o fenrir.iso

.PHONY: run
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
		-smp 1 \
		-d int

.PHONY: clean
clean:
	$(MAKE) -C kernel clean
	rm -rf ./third_party/limine/ ./kernel/bindings/flanterm/flanterm/ ./kernel/bindings/uacpi/uacpi/ ./iso_root/ fenrir.iso
