#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

cpp_args = [
    '-fno-stack-protector',
    '-fno-pic',
    '-fno-pie',
    '-fno-exceptions',
    '-fno-rtti',
    '-nostdlib',
]

nasm_args = [
    '-f', 'elf64',
    '-g',
]

link_args = [
    '-nostdlib',
    '-static',
    '-z', 'noexecstack',
]

subdir('libraries')
subdir('applications')
