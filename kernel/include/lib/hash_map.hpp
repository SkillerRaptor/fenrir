/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

template <typename K, typename V>
class HashMap {
private:
    struct Entry {
        K key { };
        V value { };
        bool is_used { false };
        bool is_deleted { false };
    };

public:
    HashMap() = default;

    ~HashMap() { delete[] m_buckets; }

    void insert(const K &key, const V &value)
    {
        if (m_size >= m_capacity / 2) {
            resize(m_capacity == 0 ? 8 : m_capacity * 2);
        }

        const usize index = find_slot(key);
        if (!m_buckets[index].is_used || m_buckets[index].is_deleted) {
            ++m_size;
        }

        m_buckets[index] = { key, value, true, false };
    }

    V *get(const K &key) const
    {
        if (m_capacity == 0) {
            return nullptr;
        }

        const usize index = Hash<K> { }(key);
        for (usize i = 0; i < m_capacity; ++i) {
            Entry &entry = m_buckets[(index + i) % m_capacity];
            if (!entry.is_used) {
                return nullptr;
            }

            if (!entry.is_deleted && entry.key == key) {
                return &entry.value;
            }
        }

        return nullptr;
    }

    bool contains(const K &key) const { return get(key) != nullptr; }

    bool remove(const K &key)
    {
        if (m_capacity == 0) {
            return false;
        }

        const usize index = Hash<K> { }(key) % m_capacity;
        for (usize i = 0; i < m_capacity; ++i) {
            Entry &entry = m_buckets[(index + i) % m_capacity];
            if (!entry.is_used) {
                return false;
            }

            if (!entry.is_deleted && entry.key == key) {
                entry.is_deleted = true;
                --m_size;
                return true;
            }
        }

        return false;
    }

    template <typename Fn>
    void for_each(const Fn &fn)
    {
        for (usize i { 0 }; i < m_capacity; ++i) {
            Entry &entry = m_buckets[i];
            if (entry.is_used && !entry.is_deleted) {
                fn(entry.key, entry.value);
            }
        }
    }

    usize size() const { return m_size; }
    usize capacity() const { return m_capacity; }

private:
    usize find_slot(const K &key) const
    {
        usize first_dead = m_capacity;

        const usize index = Hash<K> { }(key) % m_capacity;
        for (usize i { 0 }; i < m_capacity; ++i) {
            const usize j = (index + i) % m_capacity;
            Entry &entry = m_buckets[j];
            if (!entry.is_used) {
                return (first_dead < m_capacity) ? first_dead : j;
            }

            if (entry.is_deleted && first_dead == m_capacity) {
                first_dead = j;
            }

            if (!entry.is_deleted && entry.key == key) {
                return j;
            }
        }

        return first_dead;
    }

    void resize(const usize new_capacity)
    {
        Entry *old_buckets = m_buckets;
        const usize old_capacity = m_capacity;

        m_buckets = new Entry[new_capacity];

        for (usize i { 0 }; i < new_capacity; ++i) {
            m_buckets[i] = Entry { };
        }

        m_capacity = new_capacity;
        m_size = 0;

        for (usize i { 0 }; i < old_capacity; ++i) {
            Entry &entry = old_buckets[i];
            if (entry.is_used && !entry.is_deleted) {
                insert(entry.key, entry.value);
            }
        }

        delete[] old_buckets;
    }

private:
    Entry *m_buckets { nullptr };
    usize m_size { 0 };
    usize m_capacity { 0 };
};
