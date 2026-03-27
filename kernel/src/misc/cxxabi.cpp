/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "misc/cxxabi.hpp"

#include "arch/x86_64/cpu.hpp"
#include "core/types.hpp"

extern "C" {

struct AtexitFunctionEntry {
    void (*destructor)(void *);
    void *object_ptr;
    void *dso_handle;
};

static constexpr usize s_atexit_max_functions { 128 };

static usize s_atexit_function_count { 0 };
static AtexitFunctionEntry s_atexit_functions[s_atexit_max_functions] { };

void *__dso_handle { nullptr };

int __cxa_atexit(void (*destructor)(void *), void *object_ptr, void *dso_handle)
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

void __cxa_pure_virtual()
{
    // TODO: Panic here

    cpu::halt();
}

} // extern "C"

namespace cxxabi {

using ConstructorFunction = void (*)();
extern "C" ConstructorFunction __constructors_start[];
extern "C" ConstructorFunction __constructors_end[];

void construct()
{
    for (const auto *constructor = __constructors_start; constructor < __constructors_end; ++constructor) {
        (*constructor)();
    }
}

} // namespace cxxabi
