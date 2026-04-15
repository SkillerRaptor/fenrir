/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

extern long syscall(long long number, ...);

int main()
{
    syscall(0x02, 1, "Hello from Userland", 19);

    return 0;
}
