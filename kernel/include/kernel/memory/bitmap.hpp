/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/core/types.hpp"

namespace kernel {

class Bitmap {
public:
    Bitmap(u8 *data, const usize size)
        : m_data(data)
        , m_size(size)
    {
    }

    void set(usize index, bool);
    bool get(usize index) const;

    void set_data(u8 *data) { m_data = data; }
    u8 *data() const { return m_data; }

    void set_size(const usize size) { m_size = size; }
    usize size() const { return m_size; }

private:
    u8 *m_data { nullptr };
    usize m_size { 0 };
};

} // namespace kernel
