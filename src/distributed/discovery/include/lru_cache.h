#pragma once
#include <unordered_map>
#include <list>
#include <stdexcept>
#include <string>

/**
 * A simple LRU in-memory cache
 * @tparam T Type to be stored in the cache
 */
template<typename T>
class lru_cache
{
    size_t _cap;
    using list_iter = std::list<std::pair<std::string, T>>::iterator;
    std::list<std::pair<std::string, T>> _items; // MRU at front
    std::unordered_map<std::string, list_iter> _items_map;

public:
    explicit lru_cache(size_t cap = 5) : _cap(cap) {}

    bool contains(const std::string& key) const
    {
        return _items_map.contains(key);
    }

    T& get(const std::string& key)
    {
        auto it = _items_map.find(key);
        if (it == _items_map.end())
        {
            throw std::out_of_range("Key not found in lru_cache: " + key);
        }

        // Move item to front (MRU)
        _items.splice(_items.begin(), _items, it->second);
        return it->second->second;
    }

    void put(const std::string& key, const T& value)
    {
        auto it = _items_map.find(key);

        // Modify existing item
        if (it != _items_map.end())
        {
            it->second->second = value;
            _items.splice(_items.begin(), _items, it->second);
            return;
        }

        // Remove the least recently used item if cap is exceeded
        if (_items.size() >= _cap)
        {
            auto& old = _items.back();
            _items_map.erase(old.first);
            _items.pop_back();
        }

        // Insert new item
        _items.emplace_front(key, value);
        _items_map[key] = _items.begin();
    }

    void erase(const std::string& key)
    {
        auto it = _items_map.find(key);
        if (it == _items_map.end())
            return;

        _items.erase(it->second);
        _items_map.erase(it);
    }

    void clear()
    {
        _items.clear();
        _items_map.clear();
    }
};