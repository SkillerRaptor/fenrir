#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

project(
    'fenrir-userland',
    ['c', 'cpp'],
    version: '0.0.0',
    license: 'MIT',
    default_options: ['c_std=c11', 'cpp_std=c++23', 'warning_level=2'],
)

add_global_arguments(
    '-m64',
    '-march=x86-64',
    '-mabi=sysv',
    '-fno-omit-frame-pointer',
    language: ['c', 'cpp'],
)

#-------------------------------------------------------------------------------------------
# Userland
#-------------------------------------------------------------------------------------------
initramfs_files = []

subdir('applications')
subdir('libraries')
subdir('ports')

#-------------------------------------------------------------------------------------------
# Initramfs
#-------------------------------------------------------------------------------------------
generate_initramfs = find_program('../scripts/generate_initramfs.sh', required: true)

initramfs = custom_target(
    'initramfs',
    output: 'initramfs.tar',
    depends: [
        hello_world,
        doomgeneric,
    ],
    command: [
        generate_initramfs,
        meson.project_build_root(),
        '@OUTPUT@',
        meson.project_build_root() / 'initramfs',
    ]
    + [
        hello_world,
        doomgeneric,
    ],
    console: true,
    build_by_default: true,
    build_always_stale: true,
)
