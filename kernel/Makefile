#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

.SUFFIXES:

BUILD ?= debug

ifeq ($(BUILD), release)
    CARGO_FLAGS := --release
else
    CARGO_FLAGS :=
endif

TARGET_DIR := target/x86_64-fenrir/$(BUILD)

.PHONY: all
all:
	RUSTFLAGS="-C relocation-model=static" cargo build $(CARGO_FLAGS)
	nm -Cn $(TARGET_DIR)/kernel > $(TARGET_DIR)/kernel_symbols.map

.PHONY: clean
clean:
	cargo clean

