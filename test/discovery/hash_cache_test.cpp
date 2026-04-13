#include <catch2/catch_test_macros.hpp>
#include "hash_cache.h"
#include <vector>
#include <cstring>

TEST_CASE("Hash cache operations", "[hash]") {
    hash_cache cache(2);
    std::vector<std::byte> dummy(100, std::byte{0xAA});

    SECTION("hash_file stores and returns hash") {
        std::string hash = cache.hash_file("file1", dummy);
        REQUIRE_FALSE(hash.empty());
        REQUIRE(cache.has_file("file1"));
        auto cached = cache.get_file_hash("file1");
        REQUIRE(cached.has_value());
        REQUIRE(cached.value() == hash);
    }

    SECTION("get_file_hash returns nullopt for missing") {
        auto missing = cache.get_file_hash("nosuch");
        REQUIRE_FALSE(missing.has_value());
    }

    SECTION("erase removes entry") {
        cache.hash_file("to_erase", dummy);
        REQUIRE(cache.has_file("to_erase"));
        cache.erase("to_erase");
        REQUIRE_FALSE(cache.has_file("to_erase"));
    }

    SECTION("eviction by capacity") {
        cache.hash_file("f1", dummy);
        cache.hash_file("f2", dummy);
        cache.hash_file("f3", dummy); // should evict f1 (capacity 2)
        REQUIRE_FALSE(cache.has_file("f1"));
        REQUIRE(cache.has_file("f2"));
        REQUIRE(cache.has_file("f3"));
    }
}
