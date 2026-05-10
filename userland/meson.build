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
    language: ['c', 'cpp'],
)

#-------------------------------------------------------------------------------------------
# Userland
#-------------------------------------------------------------------------------------------
subdir('libraries')
subdir('applications')
subdir('ports')

#-------------------------------------------------------------------------------------------
# Initramfs
#-------------------------------------------------------------------------------------------
generate_initramfs = find_program('../scripts/generate_initramfs.sh', required: true)

initramfs = custom_target(
    'initramfs',
    input: initramfs_files,
    output: 'initramfs.tar',
    command: [
        generate_initramfs,
        '@OUTPUT@',
    ]
    + initramfs_files,
    console: true,
    build_by_default: true,
)