#pragma once

#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace rediss {

/// Thread-safe in-memory key-value store.
///
/// Uses a shared_mutex to allow concurrent reads while serializing writes.
/// This is the central data structure — every command ultimately reads from
/// or writes to this store.
class KVStore {
public:
    KVStore() = default;

    // Non-copyable, non-movable (owns a mutex)
    KVStore(const KVStore&) = delete;
    KVStore& operator=(const KVStore&) = delete;

    /// SET key value — always succeeds, overwrites any existing value.
    void set(const std::string& key, const std::string& value);

    /// GET key — returns the value if the key exists, std::nullopt otherwise.
    std::optional<std::string> get(const std::string& key) const;

    /// DEL key — removes the key. Returns true if the key existed.
    bool del(const std::string& key);

    /// EXISTS key — returns true if the key is present.
    bool exists(const std::string& key) const;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> data_;
};

} // namespace rediss
