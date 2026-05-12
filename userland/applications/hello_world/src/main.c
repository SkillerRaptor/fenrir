/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

int main(int argc, char **argv)
{
    printf("Hello from Userland!\n");

    for (int i = 0; i < argc; ++i) {
        printf("#%i: %s\n", i, argv[i]);
    }

    return 0;
}
