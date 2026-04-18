/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "syscall/syscalls.hpp"

#include "arch/x86_64/cpu.hpp"
#include "core/boot.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "lib/math.hpp"
#include "lib/string.hpp"
#include "memory/vmm.hpp"
#include "scheduler/process.hpp"
#include "syscall/syscalls/exit.hpp"

namespace syscalls {

static constexpr u32 s_star_msr = 0xc0000081;
static constexpr u32 s_lstar_msr = 0xc0000082;
static constexpr u32 s_fmask_msr = 0xc0000084;

using SyscallHandler = void (*)(const SyscallRegisters *);

extern "C" void syscall_entry();

static SyscallHandler s_syscall_handlers[256] { };

void initialize() { s_syscall_handlers[0x01] = sys$exit; }

void load()
{
    // NOTE: Enable syscall instruction
    cpu::write_msr(0xc0000080, cpu::read_msr(0xc0000080) | (1 << 0));

    cpu::write_msr(s_star_msr, (static_cast<u64>(0x30 | 3) << 48) | (static_cast<u64>(0x28) << 32));

    cpu::write_msr(s_lstar_msr, reinterpret_cast<u64>(syscall_entry));

    // NOTE: Clear IF (interrupts) and DF (direction)
    cpu::write_msr(s_fmask_msr, (1 << 10) | (1 << 9));
}

extern "C" void syscall_handler(const SyscallRegisters *registers)
{
    const u64 syscall_id = registers->rax;
    const u64 first_argument = registers->rdi;
    const u64 second_argument = registers->rsi;
    const u64 third_argument = registers->rdx;
    const u64 fourth_argument = registers->rcx;
    const u64 fifth_argument = registers->r8;

    if (s_syscall_handlers[syscall_id]) {
        s_syscall_handlers[syscall_id](registers);
        return;
    }

    switch (syscall_id) {
    case 0x02: {
        char *string = new char[second_argument];
        memcpy(string, reinterpret_cast<char *>(first_argument), second_argument);
        string[second_argument] = '\0';

        logger::info("%s\n", string);

        const limine_framebuffer *framebuffer = boot::get_framebuffers()[0];
        volatile u32 *fb_ptr = static_cast<volatile u32 *>(framebuffer->address);
        for (usize y = 0; y < framebuffer->height; y++) {
            for (usize x = 0; x < framebuffer->width; x++) {
                const u32 n_x = x * 255 / framebuffer->width;
                const u32 n_y = y * 255 / framebuffer->height;
                fb_ptr[y * (framebuffer->pitch / 4) + x] = (n_y << 8) | n_x;
            }
        }

        delete[] string;

        break;
    }
    case 0x03: {
        if (first_argument == 0) {
            u64 *count = reinterpret_cast<u64 *>(second_argument);
            *count = 1;
        } else {
            struct Device {
                const char *name;
                u64 id;

                u64 width;
                u64 height;
                u64 pitch;
            };

            Device *devices = reinterpret_cast<Device *>(first_argument);
            devices[0] = {
                .name = "Framebuffer",
                .id = 69,
                .width = boot::get_framebuffers()[0]->width,
                .height = boot::get_framebuffers()[0]->height,
                .pitch = boot::get_framebuffers()[0]->pitch,
            };

            u64 *count = reinterpret_cast<u64 *>(second_argument);
            *count = 1;
        }
        break;
    }
    case 0x04: { // Open (Ignore)
        break;
    }
    case 0x05: { // Close (Ignore)
        break;
    }
    case 0x06: {
        const cpu::Core &core = cpu::current();

        limine_framebuffer *framebuffer = boot::get_framebuffers()[0];

        const u64 physical_address = reinterpret_cast<u64>(framebuffer->address) - boot::get_hhdm_offset();
        const u64 fb_size_bytes = framebuffer->pitch * framebuffer->height;

        // round up to page boundary — the framebuffer may not end on a page boundary
        const u64 fb_pages = math::div_round_up(fb_size_bytes, memory::s_page_size);

        // choose a fixed virtual address in user space for the framebuffer
        // pick something well above typical ELF load addresses but below stack
        // 0x0000700000000000 is a reasonable choice — far from code and stack
        constexpr u64 fb_user_vaddr = 0x0000000001000000;

        // align the physical base down to a page boundary
        // the framebuffer physical address might not be page-aligned
        const u64 aligned_phys = math::align_down(physical_address, memory::s_page_size);
        const u64 phys_offset = physical_address - aligned_phys;

        // recalculate page count accounting for the alignment offset
        const u64 total_bytes = fb_size_bytes + phys_offset;
        const u64 total_pages = math::div_round_up(total_bytes, memory::s_page_size);

        for (u64 i = 0; i < total_pages; ++i) {
            vmm::map(
                core.current_thread->process->page_map,
                aligned_phys + i * memory::s_page_size,
                fb_user_vaddr + i * memory::s_page_size,
                vmm::Attribute::Write | vmm::Attribute::User);
        }

        u64 *ptr = reinterpret_cast<u64 *>(second_argument);
        *ptr = fb_user_vaddr;

        break;
    }
    default:
        logger::debug(
            "Unhandled syscall %u (rdi: %llu, rsi: %llu, rdx: %llu, rcx: %llu, r8: %llu)\n",
            syscall_id,
            first_argument,
            second_argument,
            third_argument,
            fourth_argument,
            fifth_argument);
        break;
    }
}

} // namespace syscalls
