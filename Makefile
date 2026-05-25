#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

JINX_ARCH=x86_64
JINX_BUILD_DIR=$(shell pwd)/build-$(JINX_ARCH)
JINX_DIR=$(shell pwd)/jinx

JINX_REPOSITORY="https://github.com/Mintsuki/Jinx.git"
JINX_COMMIT="97563a3705e8c2ac20941049835109d0eaad97c9"

SYSROOT_DIR=$(JINX_BUILD_DIR)/sysroot
ISO_ROOT_DIR=$(JINX_BUILD_DIR)/iso-root
LIMINE_DIR=$(JINX_BUILD_DIR)/host-pkgs/limine

JINX_STAMP=$(JINX_BUILD_DIR)/fenrir.stamp
KERNEL_STAMP=$(JINX_BUILD_DIR)/kernel.stamp
PACKAGES_STAMP=$(JINX_BUILD_DIR)/packages.stamp
SYSROOT_STAMP=$(JINX_BUILD_DIR)/sysroot.stamp

INITRAMFS=$(JINX_BUILD_DIR)/initramfs.tar
ISO=$(JINX_BUILD_DIR)/fenrir.iso

PACKAGES=doomgeneric hello_world

.PHONY: all clean run

all: $(ISO)

jinx:
	@if [ -d "$(JINX_DIR)/.git" ]; then \
		current_commit="$$(git -C "$(JINX_DIR)" rev-parse HEAD 2>/dev/null || true)"; \
		if [ "$$current_commit" != "$(JINX_COMMIT)" ]; then \
			git -C "$(JINX_DIR)" reset --hard; \
			git -C "$(JINX_DIR)" clean -fd; \
			git -C "$(JINX_DIR)" fetch origin "$(JINX_COMMIT)"; \
			git -C "$(JINX_DIR)" -c advice.detachedHead=false checkout "$(JINX_COMMIT)"; \
		fi; \
	else \
		git clone "$(JINX_REPOSITORY)" "$(JINX_DIR)"; \
		git -C "$(JINX_DIR)" -c advice.detachedHead=false checkout "$(JINX_COMMIT)"; \
	fi

$(JINX_STAMP): jinx
	mkdir -p $(JINX_BUILD_DIR)
	cd $(JINX_BUILD_DIR) && \
	../jinx/jinx init .. ARCH=$(JINX_ARCH) && \
	../jinx/jinx build host:limine
	touch $(JINX_STAMP)

KERNEL_SOURCES=$(shell find kernel -type f 2>/dev/null)

$(KERNEL_STAMP): $(JINX_STAMP) $(KERNEL_SOURCES)
	cd $(JINX_BUILD_DIR) && \
	../jinx/jinx build kernel
	touch $(KERNEL_STAMP)

PACKAGE_SOURCES=$(shell find userland -type f 2>/dev/null)

$(PACKAGES_STAMP): $(JINX_STAMP) $(PACKAGE_SOURCES)
	cd $(JINX_BUILD_DIR) && \
	../jinx/jinx build $(PACKAGES)
	touch $(PACKAGES_STAMP)

$(SYSROOT_STAMP): $(KERNEL_STAMP) $(PACKAGES_STAMP)
	mkdir -p $(SYSROOT_DIR)
	cd $(JINX_BUILD_DIR) && \
	../jinx/jinx install -f $(SYSROOT_DIR) kernel $(PACKAGES)
	touch $(SYSROOT_STAMP)

$(INITRAMFS): $(SYSROOT_STAMP)
	cd $(JINX_BUILD_DIR) && \
	tar \
		--sort=name \
		--owner=0 \
		--group=0 \
		--numeric-owner \
		--format=ustar \
		-cf ./initramfs.tar \
		-C $(SYSROOT_DIR)/staging \
		.

$(ISO): $(SYSROOT_STAMP) $(INITRAMFS)
	mkdir -p $(ISO_ROOT_DIR)/boot
	cp $(SYSROOT_DIR)/kernel $(ISO_ROOT_DIR)/boot/
	cp $(SYSROOT_DIR)/kernel_symbols.map $(ISO_ROOT_DIR)/boot/
	cp $(INITRAMFS) $(ISO_ROOT_DIR)/boot/

	mkdir -p $(ISO_ROOT_DIR)/boot/limine
	cp limine.conf $(ISO_ROOT_DIR)/boot/limine

	cp $(LIMINE_DIR)/usr/local/share/limine/limine-bios.sys $(ISO_ROOT_DIR)/boot/limine
	cp $(LIMINE_DIR)/usr/local/share/limine/limine-bios-cd.bin $(ISO_ROOT_DIR)/boot/limine
	cp $(LIMINE_DIR)/usr/local/share/limine/limine-uefi-cd.bin $(ISO_ROOT_DIR)/boot/limine

	mkdir -p $(ISO_ROOT_DIR)/EFI/BOOT
	cp $(LIMINE_DIR)/usr/local/share/limine/BOOTX64.EFI $(ISO_ROOT_DIR)/EFI/BOOT
	cp $(LIMINE_DIR)/usr/local/share/limine/BOOTIA32.EFI $(ISO_ROOT_DIR)/EFI/BOOT

	xorriso -as mkisofs \
        -R -r -J \
        -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot \
        -boot-load-size 4 \
        -boot-info-table \
        -hfsplus \
        -apm-block-size 2048 \
        --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part \
        --efi-boot-image \
        --protective-msdos-label \
        $(ISO_ROOT_DIR) -o $(ISO)

	$(LIMINE_DIR)/usr/local/bin/limine bios-install $(ISO)

clean:
	rm -rf $(shell pwd)/host-sources
	rm -rf $(shell pwd)/sources
	rm -rf $(JINX_BUILD_DIR)

run:
	qemu-system-x86_64 \
		-M q35,smm=off \
        -accel tcg \
        -boot d \
        -cdrom $(JINX_BUILD_DIR)/fenrir.iso \
        --no-reboot \
        --no-shutdown \
        -m 256M \
        -serial stdio \
        -smp 4
