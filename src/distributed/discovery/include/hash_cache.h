#pragma once
#include <optional>

#include "lru_cache.h"
#include "sha256.h"

/**
 * Uses SHA256 to hash files
 */
class hash_cache
{
    lru_cache<std::string> _lruCache;

public:
    explicit hash_cache(const size_t capacity = 50) : _lruCache(capacity)
    {
    }

    /**
     * Adds a file's hash to the cache
     * @param name Name of the file (or path)
     * @param file Bytes of the file
     * @return hash of the file
     */
    std::string hash_file(const std::string& name, const std::vector<std::byte>& file);

    /**
     * Gets a cached hash, or std::nullopt if it doesn't exist
     * @param name Name of the file (or path)
     * @return hash or nullopt
     */
    std::optional<std::string> get_file_hash(const std::string& name);

    /**
     * Checks if this hash is present in the cache
     * @param name Name of the file (or path)
     * @return true if hash is in cache
     */
    bool has_file(const std::string& name) const;

    /**
     * Erases hash from cache
     * @param name Name of the file (or path)
     */
    void erase(const std::string& name);
};
