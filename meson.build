#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

project(
    'fenrir',
    version: '0.0.0',
    license: 'MIT',
)

build_type = get_option('build_type')

cargo = find_program('cargo', required: true)
curl = find_program('curl', required: true)
git = find_program('git', required: true)
qemu = find_program('qemu-system-x86_64', required: true)
wget = find_program('wget', required: true)
xorriso = find_program('xorriso', required: true)

target_dir = meson.project_source_root() / 'target/x86_64-fenrir' / build_type

cargo_flags = build_type == 'release' ? ['--release'] : []

kernel_bin = target_dir / 'kernel'
kernel_symbols = target_dir / 'kernel_symbols.map'

clean = find_program('scripts/clean.sh', required: true)

# Clones
clone_flanterm = find_program('scripts/clone_flanterm.sh', required: true)
clone_limine = find_program('scripts/clone_limine.sh', required: true)
clone_mlibc = find_program('scripts/clone_mlibc.sh', required: true)
clone_uacpi = find_program('scripts/clone_uacpi.sh', required: true)

# Compilations
compile_binutils = find_program('scripts/compile_binutils.sh', required: true)
compile_gcc = find_program('scripts/compile_gcc.sh', required: true)
compile_kernel = find_program('scripts/compile_kernel.sh', required: true)
compile_mlibc = find_program('scripts/compile_mlibc.sh', required: true)
compile_userland = find_program('scripts/compile_userland.sh', required: true)

generate_iso = find_program('scripts/generate_iso.sh', required: true)
install_mlibc = find_program('scripts/install_mlibc.sh', required: true)

#-------------------------------------------------------------------------------------------
# Third Party
#-------------------------------------------------------------------------------------------
flanterm_clone = custom_target(
    'flanterm_clone',
    input: clone_flanterm.full_path(),
    output: 'flanterm_clone.stamp',
    command: [
        clone_flanterm,
        meson.project_source_root(),
        '@OUTPUT@',
        git.full_path(),
    ],
    console: true,
)

limine_clone = custom_target(
    'limine_clone',
    input: clone_limine.full_path(),
    output: 'limine_clone.stamp',
    command: [
        clone_limine,
        meson.project_source_root(),
        '@OUTPUT@',
        curl.full_path(),
        wget.full_path(),
    ],
    console: true,
)

mlibc_clone = custom_target(
    'mlibc_clone',
    input: clone_mlibc.full_path(),
    output: 'mlibc_clone.stamp',
    command: [clone_mlibc, meson.project_source_root(), '@OUTPUT@', git.full_path()],
    console: true,
    build_always_stale: true,
)

uacpi_clone = custom_target(
    'uacpi_clone',
    input: clone_uacpi.full_path(),
    output: 'uacpi_clone.stamp',
    command: [clone_uacpi, meson.project_source_root(), '@OUTPUT@', git.full_path()],
    console: true,
)

#-------------------------------------------------------------------------------------------
# Kernel
#-------------------------------------------------------------------------------------------
kernel_dir = meson.project_source_root() / 'kernel'

kernel_sources = run_command(
    'find',
    kernel_dir,
    '-name', '*.rs',
    '-o',
    '-name', '*.asm',
    '-o',
    '-name', '*.ld',
    '-o',
    '-name', '*.toml',
    check: true,
).stdout().strip().split('\n') + [
    meson.project_source_root() / 'Cargo.toml',
    meson.project_source_root() / 'Cargo.lock',
]

kernel = custom_target(
    'kernel',
    input: kernel_sources,
    output: 'kernel.stamp',
    depends: [flanterm_clone, uacpi_clone],
    command: [
        compile_kernel,
        meson.project_source_root(),
        '@OUTPUT@',
        cargo.full_path(),
        kernel_bin,
        kernel_symbols,
    ]
    + cargo_flags,
    console: true,
)

#-------------------------------------------------------------------------------------------
# Toolchain
#-------------------------------------------------------------------------------------------
mlibc_install = custom_target(
    'mlibc_install',
    output: 'mlibc_install.stamp',
    depends: [mlibc_clone],
    command: [
        install_mlibc,
        meson.project_source_root(),
        '@OUTPUT@',
    ],
    console: true,
)

binutils = custom_target(
    'binutils',
    input: meson.project_source_root() / 'patches/binutils.patch',
    output: 'binutils.stamp',
    depends: [mlibc_install],
    command: [
        compile_binutils,
        meson.project_source_root(),
        '@OUTPUT@',
        wget.full_path(),
    ],
    console: true,
)

gcc = custom_target(
    'gcc',
    input: meson.project_source_root() / 'patches/gcc.patch',
    output: 'gcc.stamp',
    depends: [mlibc_install, binutils],
    command: [
        compile_gcc,
        meson.project_source_root(),
        '@OUTPUT@',
        wget.full_path(),
    ],
    console: true,
)

mlibc = custom_target(
    'mlibc',
    input: meson.project_source_root() / 'patches/mlibc.patch',
    output: 'mlibc.stamp',
    depends: [mlibc_install, gcc],
    command: [
        compile_mlibc,
        meson.project_source_root(),
        '@OUTPUT@',
    ],
    console: true,
    build_by_default: true,
)

#-------------------------------------------------------------------------------------------
# Userland
#-------------------------------------------------------------------------------------------
userland = custom_target(
    'userland',
    input: run_command(
        'find',
        meson.project_source_root() / 'userland',
        '-name', '*.c',
        '-o',
        '-name', '*.cpp',
        '-o',
        '-name', '*.h',
        '-o',
        '-name', '*.hpp',
        '-o',
        '-name', '*.asm',
        '-o',
        '-name', '*.build',
        check: true,
    ).stdout().strip().split('\n'),
    output: 'userland.stamp',
    depends: [mlibc],
    command: [
        compile_userland,
        meson.project_source_root(),
        meson.project_build_root(),
        '@OUTPUT@',
    ],
    console: true,
)

#-------------------------------------------------------------------------------------------
# Iso
#-------------------------------------------------------------------------------------------
limine_dir = meson.project_source_root() / 'third_party/limine'

iso_image = custom_target(
    'fenrir.iso',
    output: 'fenrir.iso',
    depends: [limine_clone, kernel, userland],
    command: [
        generate_iso,
        meson.project_source_root(),
        '@OUTPUT@',
        xorriso.full_path(),
        kernel_bin,
        kernel_symbols,
        meson.project_build_root() / 'userland/initramfs.tar',
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
