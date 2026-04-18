/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include <leviathan/syscall.h>

int main()
{
    write("Hello from Userland", 19);

    return 0;
}
