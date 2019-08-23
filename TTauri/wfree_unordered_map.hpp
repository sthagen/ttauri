// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <atomic>
#include <array>
#include <optional>

namespace TTauri {

template<typename K, typename V>
struct wait_free_unordered_map_item {
    /*! The value.
     * It comes first because it is of unknown size
     * at should be aligned to 128 bits so it stays atomic
     * up to 128 bits in size.
     */
    alignas(16) std::atomic<V> value = {};

    /*! Hash for quick comparison and for state.
     * Special values:
     *  * 0 = Empty
     *  * 1 = Busy
     *  * 2 = Tombstone
     *
     * Natural hash values 0, 1, 2 must be mapped to 3, 4, 5.
     */
    std::atomic<size_t> hash = 0;
    K key = {};

    constexpr wait_free_unordered_map_item() noexcept = default;
    constexpr wait_free_unordered_map_item(wait_free_unordered_map_item const &) noexcept = default;
    constexpr wait_free_unordered_map_item(wait_free_unordered_map_item &&) noexcept = default;
    constexpr ~wait_free_unordered_map_item() noexcept = default;
    constexpr wait_free_unordered_map_item &operator=(wait_free_unordered_map_item const &) noexcept = default;
    constexpr wait_free_unordered_map_item &operator=(wait_free_unordered_map_item &&) noexcept = default;
};

/*! Unordered map with wait-free insert, get and erase.
 *
 * This class can be instantiated as a global variable without
 * needing initialization.
 */
template<size_t MAX_NR_ITEMS,typename K, typename V>
class wfree_unordered_map {
public:
    using key_type = K;
    using mapped_type = V;

private:
    static constexpr size_t CAPACITY = MAX_NR_ITEMS * 2;

    std::array<wait_free_unordered_map_item<K,V>, CAPACITY> items = {};

public:
    constexpr wait_free_unordered_map() noexcept = default;
    constexpr wait_free_unordered_map(wait_free_unordered_map const &) noexcept = default;
    constexpr wait_free_unordered_map(wait_free_unordered_map &&) noexcept = default;
    constexpr ~wait_free_unordered_map() noexcept = default;
    constexpr wait_free_unordered_map &operator=(wait_free_unordered_map const &) noexcept = default;
    constexpr wait_free_unordered_map &operator=(wait_free_unordered_map &&) noexcept = default;

    static size_t make_hash(K const &value) noexcept {
        let hash = std::hash<K>{}(key);
        return hash >= 3 ? hash : hash + 3;
    }

    void insert(K key, V value) noexcept {
        let hash = make_hash(key);

        auto index = hash % CAPACITY;
        while (true) {
            auto &item = items[index];

            // First look for an empty (0) item, highly likely when doing insert.
            size_t item_hash = 0;
            if (item.hash.compare_exchange_strong(item_hash, 1, std::memory_order_acquire)) {
                // Success, we found an empty entry, we marked it as busy (1).
                item.key = std::move(key);
                item.value.store(value, std::memory_order_relaxed);
                item.hash.store(hash, std::memory_order_release);
                return;

            } else if (item_hash == hash && key == item.key) {
                // Key was already in map, replace the value.
                item.value.store(value, std::memory_order_release);
                return;

            } else {
                // Either this item was already in use by another key, or
                // Another thread was ahead of us with claiming this item (hopefully the other
                // thread doesn't insert the same key).
                // Even though compare_exchange was used here, this algorithm is wait-free since all threads
                // including the one here are making normal progress.
                index = (index + 1) % CAPACITY;
            }
        }
    }

    std::optional<V> get(K const &key) const noexcept {
        let hash = make_hash(key);

        auto index = hash % CAPACITY;
        while (true) {
            auto &item = items[index];

            auto item_hash = item.hash.load(std::memory_order_acquire);

            if (item_hash == hash && key == item.key) {
                // Found key
                return { item.value.load(std::memory_order_relaxed) };

            } else if (item_hash == 0) {
                // Item is empty.
                return {};

            } else {
                index = (index + 1) % CAPACITY;
            }
        }
    }

    std::optional<V> erase(K const &key) noexcept {
        let hash = make_hash(key);

        auto index = hash % CAPACITY;
        while (true) {
            auto &item = items[index];
            auto item_hash = item.hash.load(std::memory_order_acquire);

            if (item_hash == hash && key == item.key) {
                // Set tombstone. Don't actually delete the key or value.
                item.hash.store(1, std::memory_order_release);
                return { item.value.load(std::memory_order_relaxed) };

            } else if (item_hash == 0) {
                // Item is empty.
                return {};

            } else {
                index = (index + 1) % CAPACITY;
            }
        }
    }
};


}
