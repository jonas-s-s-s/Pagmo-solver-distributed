#include <catch2/catch_test_macros.hpp>
#include "distributed/discovery/include/hash_cache.h"
#include <vector>
#include <string>

TEST_CASE("hash_cache: hashing, caching, get, has, erase, eviction", "[hash]") {
    hash_cache cache(2);

    std::vector<std::byte> data1{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    std::vector<std::byte> data2{std::byte{0x04}, std::byte{0x05}};
    std::vector<std::byte> data3{std::byte{0x06}};

    SECTION("hash_file stores and get_file_hash retrieves") {
        std::string h1 = cache.hash_file("file1.bin", data1);
        REQUIRE(cache.has_file("file1.bin"));
        auto opt = cache.get_file_hash("file1.bin");
        REQUIRE(opt.has_value());
        REQUIRE(opt.value() == h1);
        std::string h1_again = cache.hash_file("file1.bin", data1);
        REQUIRE(h1 == h1_again);
    }

    SECTION("eviction via capacity") {
        cache.hash_file("f1", data1);
        cache.hash_file("f2", data2);
        cache.hash_file("f3", data3);
        REQUIRE_FALSE(cache.has_file("f1"));
        REQUIRE(cache.has_file("f2"));
        REQUIRE(cache.has_file("f3"));
    }

    SECTION("erase works") {
        cache.hash_file("erase_me", data1);
        REQUIRE(cache.has_file("erase_me"));
        cache.erase("erase_me");
        REQUIRE_FALSE(cache.has_file("erase_me"));
        REQUIRE_FALSE(cache.get_file_hash("erase_me").has_value());
    }
}
