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

#-------------------------------------------------------------------------------------------
# Options
#-------------------------------------------------------------------------------------------
build_type = get_option('build_type')

#-------------------------------------------------------------------------------------------
# Programs
#-------------------------------------------------------------------------------------------
cargo = find_program('cargo', required: true)
curl = find_program('curl', required: true)
git = find_program('git', required: true)
qemu = find_program('qemu-system-x86_64', required: true)
wget = find_program('wget', required: true)
xorriso = find_program('xorriso', required: true)

#-------------------------------------------------------------------------------------------
# Third Party
#-------------------------------------------------------------------------------------------
clone_flanterm = find_program('scripts/clone_flanterm.sh', required: true)
clone_limine = find_program('scripts/clone_limine.sh', required: true)
clone_uacpi = find_program('scripts/clone_uacpi.sh', required: true)

flanterm_commit = 'dfa1922c0749faf64c4423a81b233a20c08a7db7'
limine_tag = 'v12.2.0'
uacpi_commit = '71f981d4ee36bf3ae85976409734000578b5249c'

flanterm_clone = custom_target(
    'flanterm_clone',
    output: 'flanterm_clone.stamp',
    command: [
        clone_flanterm,
        meson.project_source_root(),
        '@OUTPUT@',
        git.full_path(),
        flanterm_commit,
    ],
    console: true,
    build_by_default: true,
    build_always_stale: true,
)

limine_clone = custom_target(
    'limine_clone',
    output: 'limine_clone.stamp',
    command: [
        clone_limine,
        meson.project_source_root(),
        '@OUTPUT@',
        curl.full_path(),
        wget.full_path(),
        limine_tag,
    ],
    console: true,
    build_by_default: true,
    build_always_stale: true,
)

uacpi_clone = custom_target(
    'uacpi_clone',
    output: 'uacpi_clone.stamp',
    command: [
        clone_uacpi,
        meson.project_source_root(),
        '@OUTPUT@',
        git.full_path(),
        uacpi_commit,
    ],
    console: true,
    build_by_default: true,
    build_always_stale: true,
)

#-------------------------------------------------------------------------------------------
# Toolchain
#-------------------------------------------------------------------------------------------
compile_binutils = find_program('scripts/compile_binutils.sh', required: true)
compile_gcc = find_program('scripts/compile_gcc.sh', required: true)
compile_mlibc = find_program('scripts/compile_mlibc.sh', required: true)
install_mlibc = find_program('scripts/install_mlibc.sh', required: true)

mlibc_commit = 'd5efaaaffcb7622429986edb835792ba7d612380'

mlibc_install = custom_target(
    'mlibc_install',
    output: 'mlibc_install.stamp',
    command: [
        install_mlibc,
        meson.project_source_root(),
        meson.project_build_root(),
        '@OUTPUT@',
        git.full_path(),
        mlibc_commit,
    ],
    console: true,
    build_by_default: true,
    build_always_stale: true,
)

binutils = custom_target(
    'binutils',
    output: 'binutils.stamp',
    depends: [mlibc_install],
    command: [
        compile_binutils,
        meson.project_source_root(),
        meson.project_build_root(),
        '@OUTPUT@',
        wget.full_path(),
    ],
    console: true,
    build_by_default: true,
    build_always_stale: true,
)

gcc = custom_target(
    'gcc',
    output: 'gcc.stamp',
    depends: [mlibc_install, binutils],
    command: [
        compile_gcc,
        meson.project_source_root(),
        meson.project_build_root(),
        '@OUTPUT@',
        wget.full_path(),
    ],
    console: true,
    build_by_default: true,
    build_always_stale: true,
)

mlibc = custom_target(
    'mlibc',
    output: 'mlibc.stamp',
    depends: [mlibc_install, gcc],
    command: [
        compile_mlibc,
        meson.project_source_root(),
        meson.project_build_root(),
        '@OUTPUT@',
    ],
    console: true,
    build_by_default: true,
    build_always_stale: true,
)

#-------------------------------------------------------------------------------------------
# Kernel
#-------------------------------------------------------------------------------------------
compile_kernel = find_program('scripts/compile_kernel.sh', required: true)

cargo_flags = build_type == 'release' ? ['--release'] : []

kernel_dir = meson.project_source_root() / 'kernel'
target_dir = meson.project_source_root() / 'target/x86_64-fenrir' / build_type

kernel_bin = target_dir / 'kernel'
kernel_symbols = target_dir / 'kernel_symbols.map'

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
# Userland
#-------------------------------------------------------------------------------------------
clone_doomgeneric = find_program('scripts/clone_doomgeneric.sh', required: true)
compile_userland = find_program('scripts/compile_userland.sh', required: true)

doomgeneric_commit = 'dcb7a8dbc7a16ce3dda29382ac9aae9d77d21284'

doomgeneric_clone = custom_target(
    'doomgeneric_clone',
    output: 'doomgeneric_clone.stamp',
    command: [
        clone_doomgeneric,
        meson.project_source_root(),
        meson.project_build_root(),
        '@OUTPUT@',
        git.full_path(),
        wget.full_path(),
        doomgeneric_commit,
    ],
    console: true,
    build_by_default: true,
    build_always_stale: true,
)

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
    depends: [
        mlibc,
        doomgeneric_clone,
    ],
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
generate_iso = find_program('scripts/generate_iso.sh', required: true)

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
        '-smp', '1',
    ],
)

run_target(
    'debug',
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
        '-smp', '1',
        '-s',
        '-S',
    ],
)

#-------------------------------------------------------------------------------------------
# Clean
#-------------------------------------------------------------------------------------------
clean = find_program('scripts/clean.sh', required: true)

run_target(
    'clean-all',
    command: [
        clean,
        cargo.full_path(),
        meson.project_source_root(),
        meson.project_build_root(),
    ],
)
