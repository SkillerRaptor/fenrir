#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

src = ['src/crt0.asm', 'src/syscall.asm']

sources = files(src)
includes = include_directories('include')

extra_cpp_args = [
    '-ffreestanding',
    '-nostdinc',
]

libc = static_library(
    'libc',
    sources: sources,
    include_directories: includes,
    cpp_args: cpp_args + extra_cpp_args,
    nasm_args: nasm_args,
    link_args: link_args,
)

dep_libc = declare_dependency(
    link_with: libc,
    include_directories: includes,
)
