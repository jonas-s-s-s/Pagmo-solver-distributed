#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <optional>
#include "distributed/discovery/include/hash_cache.h"

TEST_CASE("hash_cache stores and retrieves hashes", "[hash]") {
    hash_cache cache(3);
    std::vector<std::byte> data1 = {std::byte(0x01), std::byte(0x02)};
    std::string h1 = cache.hash_file("file1.bin", data1);
    REQUIRE_FALSE(h1.empty());
    auto got = cache.get_file_hash("file1.bin");
    REQUIRE(got.has_value());
    REQUIRE(got.value() == h1);
    REQUIRE(cache.has_file("file1.bin"));
}

TEST_CASE("hash_cache eviction and erase", "[hash]") {
    hash_cache cache(2);
    std::vector<std::byte> d{std::byte(1)};
    cache.hash_file("f1", d);
    cache.hash_file("f2", d);
    cache.hash_file("f3", d); // evicts f1
    REQUIRE_FALSE(cache.has_file("f1"));
    REQUIRE(cache.has_file("f3"));
    cache.erase("f2");
    REQUIRE_FALSE(cache.has_file("f2"));
}
