#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

project(
    'fenrir',
    'c',
    license: 'MIT',
    version: '0.1.0',
    meson_version: '>=1.5.0',
    default_options: [
        'c_std=gnu23',
        'warning_level=2',
        'optimization=0',
        'debug=true',
        'werror=true',
        'b_staticpic=false',
        'b_pie=false',
        'b_lto=false',
        'b_lundef=false',
        'strip=true',
    ],
)

sources_dir = get_option('sources_dir')

flanterm_dependency = declare_dependency(
    sources: files(
        sources_dir / 'flanterm/src/flanterm.c',
        sources_dir / 'flanterm/src/flanterm_backends/fb.c',
    ),
    include_directories: include_directories(sources_dir / 'flanterm/src')
)

limine_dependency = declare_dependency(
    include_directories: include_directories(sources_dir / 'limine-headers/include')
)

nanoprintf_dependency = declare_dependency(
    include_directories: include_directories(sources_dir / 'nanoprintf')
)

fenrir_dependencies = [
    flanterm_dependency,
    limine_dependency,
    nanoprintf_dependency,
]

fenrir_c_args = [
    '-ffreestanding',
    '-fno-common',
    '-fno-stack-protector',
    '-fno-stack-check',
    '-fno-PIC',
    '-ffunction-sections',
    '-fdata-sections',
    '-m64',
    '-march=x86-64',
    '-mgeneral-regs-only',
    '-mno-red-zone',
    '-mcmodel=kernel',
]

linker_script = files('linker_scripts/x86_64.ld')

fenrir_link_args = [
    '-nostdlib',
    '-static',
    '-z', 'max-page-size=0x1000',
    '-Wl,-z,noexecstack',
    '-Wl,--gc-sections',
    '-Wl,-T', linker_script[0].full_path(),
    '-Wl,-m,elf_x86_64',
]

fenrir_sources = files(
    'src/main.c',
    'src/boot/boot.c',
    'src/lib/console.c',
    'src/lib/log.c',
    'src/lib/stdio.c',
    'src/lib/string.c',
)

fenrir_includes = include_directories('include')

fenrir = executable(
    'fenrir',
    fenrir_sources,
    c_args: fenrir_c_args,
    link_args: fenrir_link_args,
    link_depends: linker_script,
    include_directories: fenrir_includes,
    dependencies: fenrir_dependencies,
)
