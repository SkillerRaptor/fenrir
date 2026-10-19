#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

.PHONY: all clean run

all: $(ISO)

.PHONY: bootstrap
bootstrap:
	./scripts/bootstrap.sh

.PHONY: initramfs
initramfs:
	ARCH=x86_64 ./scripts/generate-initramfs.sh

.PHONY: iso
iso:
	ARCH=x86_64 ./scripts/build-kernel.sh
	./scripts/generate-iso.sh

run:
	ARCH=x86_64 ./scripts/build-kernel.sh
	./scripts/generate-iso.sh
	qemu-system-x86_64 \
		-M q35,smm=off \
        -accel tcg \
        -boot d \
        -cdrom ./fenrir.iso \
        --no-reboot \
        --no-shutdown \
        -m 256M \
        -serial stdio \
        -smp 4
