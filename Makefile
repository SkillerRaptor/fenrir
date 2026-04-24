#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

.SUFFIXES:

.PHONY: all
all: fenrir.iso

.PHONY: kernel
kernel: flanterm
	$(MAKE) -C kernel

flanterm:
ifeq ($(wildcard ./kernel/bindings/flanterm/flanterm),)
		git clone https://github.com/mintsuki/flanterm.git ./kernel/bindings/flanterm/flanterm --branch=trunk --depth=1
endif

limine:
	mkdir -p ./third_party
ifeq ($(wildcard ./third_party/limine),)
		git clone https://github.com/Limine-Bootloader/Limine.git ./third_party/limine --branch=v11.x-binary --depth=1
		$(MAKE) -C ./third_party/limine
endif

fenrir.iso: limine kernel
	mkdir -p ./iso_root/boot
	cp -v ./kernel/target/x86_64-fenrir/debug/kernel ./iso_root/boot/

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
	rm -rf ./third_party/limine/ ./kernel/bindings/flanterm/flanterm/ ./iso_root/ fenrir.iso
