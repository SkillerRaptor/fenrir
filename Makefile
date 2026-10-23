#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

.PHONY: all
all: bootstrap iso

.PHONY: bootstrap
bootstrap:
	./scripts/bootstrap.sh

.PHONY: kernel iso
kernel iso:
	./scripts/build.sh $@

.PHONY: run
run: kernel iso
	qemu-system-x86_64 \
		-M q35,smm=off \
        -accel tcg \
        -boot d \
        -cdrom ./build/iso/fenrir.iso \
        --no-reboot \
        --no-shutdown \
        -m 256M \
        -serial stdio \
        -smp 4
