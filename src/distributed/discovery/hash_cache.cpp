#include "hash_cache.h"

void hash_cache::hash_file(const std::string& name, const std::vector<std::byte>& file)
{
    SHA256 sha256;
    const std::string hash = sha256(file.data(), file.size());
    _lruCache.put(name, hash);
}

std::optional<std::string> hash_cache::get_file_hash(const std::string& name)
{
    if (_lruCache.contains(name))
    {
        return _lruCache.get(name);
    }

    return std::nullopt;
}

bool hash_cache::has_file(const std::string& name) const
{
    return _lruCache.contains(name);
}
