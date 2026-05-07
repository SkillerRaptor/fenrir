#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

c_args = [
    '-m64',
    '-march=x86-64',
    '-mabi=sysv',
    '-fno-stack-protector',
    '-fno-pic',
    '-fno-pie',
    '-nostdlib',
    '-nostdinc',
    '-Werror=implicit-function-declaration',
    '-Werror=return-type',
    '-DFENRIR=1'
]

cpp_args = [
    c_args,
    '-nostdlib',
    '-nostdinc',
]

nasm_args = [
    '-f', 'elf64',
    '-g',
]

link_args = [
    '-nostdlib',
    '-static',
    '-z', 'noexecstack',
    '-z', 'max-page-size=0x1000',
]

subdir('libraries')
subdir('applications')
