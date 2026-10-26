/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/x86_64/gdt.h"
#include "boot/boot.h"
#include "lib/console.h"
#include "lib/stdio.h"

static void hcf(void)
{
    for (;;) {
        asm("hlt");
    }
}

void kmain(void)
{
    if (!boot_is_base_revision_supported()) {
        hcf();
    }

    console_init();

    printf("\n");
    printf("   _______ _______ __   _  ______ _____  ______\n");
    printf("   |______ |______ | \\  | |_____/   |   |_____/\n");
    printf("   |       |______ |  \\_| |    \\_ __|__ |    \\_\n");
    printf("\n");
    printf("   Bootloader: %s %s\n", boot_get_bootloader_name(), boot_get_bootloader_version());
    printf("   Firmware: %s\n", boot_get_firmware_type());
    printf("\n");

    gdt_init();

    hcf();
}
