/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ygg/types.hpp"

namespace ygg {

class Bitmap {
public:
    Bitmap() = default;
    ~Bitmap() = default;

    Bitmap(const Bitmap &) = delete;
    Bitmap &operator=(const Bitmap &) = delete;

    Bitmap(Bitmap &&other) noexcept
    {
        m_data = other.m_data;
        other.m_data = nullptr;

        m_size = other.m_size;
        other.m_size = 0;
    }

    Bitmap &operator=(Bitmap &&other) noexcept
    {
        if (this == &other) {
            return *this;
        }

        m_data = other.m_data;
        other.m_data = nullptr;

        m_size = other.m_size;
        other.m_size = 0;

        return *this;
    }

    void set(usize index, bool);
    bool get(usize index) const;

    void set_data(u8 *data) { m_data = data; }
    u8 *data() const { return m_data; }

    void set_size(const usize size) { m_size = size; }
    usize size() const { return m_size; }

private:
    u8 *m_data = nullptr;
    usize m_size = 0;
};

} // namespace ygg
