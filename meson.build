#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

project(
    'fenrir',
    ['c', 'cpp', 'nasm'],
    version: '0.0.0',
    license: 'MIT',
    default_options: [
        'c_std=c11',
        'cpp_std=c++23',
        'warning_level=2',
    ],
)

build_type = get_option('build_type')

cargo = find_program('cargo', required: true)
curl = find_program('curl', required: true)
git = find_program('git', required: true)
qemu = find_program('qemu-system-x86_64', required: true)
xorriso = find_program('xorriso', required: true)

target_dir = meson.project_source_root() / 'target/x86_64-fenrir' / build_type

cargo_flags = build_type == 'release' ? ['--release'] : []

kernel_bin = target_dir / 'kernel'
kernel_symbols = target_dir / 'kernel_symbols.map'

build_kernel = find_program('scripts/build_kernel.sh', required: true)
clean = find_program('scripts/clean.sh', required: true)
clone_flanterm = find_program('scripts/clone_flanterm.sh', required: true)
clone_uacpi = find_program('scripts/clone_uacpi.sh', required: true)
download_limine = find_program('scripts/download_limine.sh', required: true)
generate_initramfs = find_program('scripts/generate_initramfs.sh', required: true)
generate_iso = find_program('scripts/generate_iso.sh', required: true)

#-------------------------------------------------------------------------------------------
# Third Party
#-------------------------------------------------------------------------------------------
limine_stamp = custom_target(
    'limine',
    output: 'limine.stamp',
    command: [
        download_limine,
        curl.full_path(),
        meson.project_source_root(),
        '@OUTPUT@',
    ],
    build_by_default: true,
)

flanterm_stamp = custom_target(
    'flanterm',
    output: 'flanterm.stamp',
    command: [
        clone_flanterm,
        git.full_path(),
        meson.project_source_root(),
        '@OUTPUT@',
    ],
    build_by_default: true,
)

uacpi_stamp = custom_target(
    'uacpi',
    output: 'uacpi.stamp',
    command: [
        clone_uacpi,
        git.full_path(),
        meson.project_source_root(),
        '@OUTPUT@',
    ],
    build_by_default: true,
)

#-------------------------------------------------------------------------------------------
# Kernel
#-------------------------------------------------------------------------------------------
kernel_sources = run_command(
    'find',
    meson.project_source_root() / 'kernel',
    '-name', '*.rs',
    check: true,
).stdout().strip().split('\n') + [
    meson.project_source_root() / 'Cargo.toml',
    meson.project_source_root() / 'Cargo.lock',
]

kernel_target = custom_target(
    'kernel',
    input: kernel_sources,
    output: 'kernel.stamp',
    depends: [flanterm_stamp, uacpi_stamp],
    command: [
        build_kernel,
        cargo.full_path(),
        meson.project_source_root(),
        '@OUTPUT@',
        kernel_bin,
        kernel_symbols,
    ]
    + cargo_flags,
    console: true,
    build_by_default: true,
    build_always_stale: true,
)

#-------------------------------------------------------------------------------------------
# Userland
#-------------------------------------------------------------------------------------------
initramfs_files = []

subdir('userland')

#-------------------------------------------------------------------------------------------
# Initramfs
#-------------------------------------------------------------------------------------------
initramfs = custom_target(
    'initramfs',
    input: initramfs_files,
    output: 'initramfs.tar',
    command: [
        generate_initramfs,
        '@OUTPUT@',
        meson.project_build_root() / 'initramfs',
    ]
    + initramfs_files,
    console: true,
    build_by_default: true,
)

#-------------------------------------------------------------------------------------------
# Iso
#-------------------------------------------------------------------------------------------
limine_dir = meson.project_source_root() / 'third_party/limine'

iso_image = custom_target(
    'fenrir.iso',
    output: 'fenrir.iso',
    depends: [kernel_target, initramfs, limine_stamp],
    command: [
        generate_iso,
        xorriso.full_path(),
        meson.project_source_root(),
        '@OUTPUT@',
        kernel_bin,
        kernel_symbols,
        initramfs,
    ],
    build_by_default: true,
)

#-------------------------------------------------------------------------------------------
# Run
#-------------------------------------------------------------------------------------------
run_target(
    'run',
    depends: [iso_image],
    command: [
        qemu,
        '-M', 'q35,smm=off',
        '-accel', 'tcg',
        '-boot', 'd',
        '-cdrom', iso_image,
        '--no-reboot',
        '--no-shutdown',
        '-m', '256M',
        '-serial', 'stdio',
        '-smp', '4',
    ],
)

#-------------------------------------------------------------------------------------------
# Clean
#-------------------------------------------------------------------------------------------
run_target(
    'clean-all',
    command: [
        clean,
        cargo.full_path(),
        meson.project_source_root(),
        meson.project_build_root(),
    ],
)