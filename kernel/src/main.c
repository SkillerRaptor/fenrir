/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/cpu.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "boot/boot.h"
#include "lib/console.h"
#include "lib/log.h"
#include "lib/stacktrace.h"
#include "lib/stdio.h"
#include "mem/pmm.h"
#include "mem/vmm.h"

[[noreturn]] void kmain()
{
    if (!boot_is_base_revision_supported()) {
        cpu_hcf();
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
    idt_init();

    pmm_init();
    vmm_init();

    stacktrace_init();

    LOG_INFO("Fenrir successfully booted!\n");

    cpu_hcf();
}
