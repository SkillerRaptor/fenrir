/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "boot/boot.h"
#include "lib/console.h"
#include "lib/log.h"

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

    hcf();
}
