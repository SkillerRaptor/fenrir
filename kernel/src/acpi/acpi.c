/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "acpi/acpi.h"

#include <uacpi/sleep.h>
#include <uacpi/uacpi.h>

#include "lib/log.h"

void acpi_init()
{
    uacpi_status ret = uacpi_initialize(0);
    if (uacpi_unlikely_error(ret)) {
        LOG_ERROR("ACPI: uacpi_initialize failed: %s\n", uacpi_status_to_string(ret));
        return;
    }

    LOG_INFO("ACPI: Initialized\n");
}
