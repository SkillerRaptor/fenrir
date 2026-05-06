#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

src = ['src/main.c']

sources = files(src)
includes = include_directories('include')

executable(
    'hello_world',
    sources: sources,
    include_directories: includes,
    dependencies: [dep_libc],
    cpp_args: cpp_args,
    link_args: link_args,
)
