/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "misc/cxxabi.hpp"

#include "arch/x86_64/cpu.hpp"
#include "lib/types.hpp"
#include "memory/kmalloc.hpp"

namespace kernel::cxxabi {

struct AtexitFunctionEntry {
    void (*destructor)(void *);
    void *object_ptr;
    void *dso_handle;
};

static constexpr usize s_atexit_max_functions { 128 };

static usize s_atexit_function_count { 0 };
static AtexitFunctionEntry s_atexit_functions[s_atexit_max_functions] {};

extern "C" void *__dso_handle { nullptr };

extern "C" int __cxa_atexit(void (*destructor)(void *), void *object_ptr, void *dso_handle)
{
    if (s_atexit_function_count >= s_atexit_max_functions) {
        return -1;
    }

    s_atexit_functions[s_atexit_function_count].destructor = destructor;
    s_atexit_functions[s_atexit_function_count].object_ptr = object_ptr;
    s_atexit_functions[s_atexit_function_count].dso_handle = dso_handle;
    ++s_atexit_function_count;

    return 0;
}

extern "C" void __cxa_pure_virtual()
{
    // TODO: Panic here

    cpu::halt();
}

using ConstructorFunction = void (*)();
extern "C" ConstructorFunction __constructors_start[];
extern "C" ConstructorFunction __constructors_end[];

void construct()
{
    for (const ConstructorFunction *constructor = __constructors_start; constructor < __constructors_end;
         ++constructor) {
        (*constructor)();
    }
}

} // namespace kernel::cxxabi

[[nodiscard]] void *operator new(const usize size) { return kernel::memory::kmalloc(size); }
[[nodiscard]] void *operator new[](const usize size) { return kernel::memory::kmalloc(size); }

void operator delete(void *ptr) noexcept { kernel::memory::kfree(ptr); }
void operator delete(void *ptr, usize) noexcept { kernel::memory::kfree(ptr); }

void operator delete[](void *ptr) noexcept { kernel::memory::kfree(ptr); }
void operator delete[](void *ptr, usize) noexcept { kernel::memory::kfree(ptr); }
