#include <catch2/catch_test_macros.hpp>
#include "hash_cache.h"

TEST_CASE("Hash Cache - hash_file operation", "[hash]") {
    hash_cache cache(5);
    
    SECTION("hash_file returns non-empty hash") {
        std::vector<std::byte> data(20);
        std::string hash = cache.hash_file("file1", data);
        
        REQUIRE(!hash.empty());
        REQUIRE(hash.length() == 64);
    }
    
    SECTION("hash_file stores in cache") {
        std::vector<std::byte> data(20);
        cache.hash_file("file1", data);
        
        REQUIRE(cache.has_file("file1"));
    }
    
    SECTION("same content produces same hash") {
        std::vector<std::byte> content{std::byte(0xAA), std::byte(0xBB)};
        
        std::string hash1 = cache.hash_file("f1", content);
        std::string hash2 = cache.hash_file("f2", content);
        
        REQUIRE(hash1 == hash2);
    }
    
    SECTION("different content produces different hash") {
        std::vector<std::byte> data1(20, std::byte(0x01));
        std::vector<std::byte> data2(20, std::byte(0x02));
        
        std::string hash1 = cache.hash_file("f1", data1);
        std::string hash2 = cache.hash_file("f2", data2);
        
        REQUIRE(hash1 != hash2);
    }
}

TEST_CASE("Hash Cache - get_file_hash operation", "[hash]") {
    hash_cache cache(5);
    std::vector<std::byte> data(20);
    
    SECTION("get_file_hash returns cached value") {
        std::string hash1 = cache.hash_file("file1", data);
        auto hash2 = cache.get_file_hash("file1");
        
        REQUIRE(hash2.has_value());
        REQUIRE(hash1 == hash2.value());
    }
    
    SECTION("get_file_hash returns nullopt for missing") {
        auto result = cache.get_file_hash("missing");
        REQUIRE(!result.has_value());
    }
}

TEST_CASE("Hash Cache - has_file operation", "[hash]") {
    hash_cache cache(5);
    std::vector<std::byte> data(20);
    
    SECTION("has_file true for cached") {
        cache.hash_file("file1", data);
        REQUIRE(cache.has_file("file1"));
    }
    
    SECTION("has_file false for missing") {
        REQUIRE(!cache.has_file("missing"));
    }
}

TEST_CASE("Hash Cache - erase operation", "[hash]") {
    hash_cache cache(5);
    std::vector<std::byte> data(20);
    
    SECTION("erase removes from cache") {
        cache.hash_file("file1", data);
        cache.erase("file1");
        
        REQUIRE(!cache.has_file("file1"));
    }
    
    SECTION("erase non-existent is safe") {
        REQUIRE_NOTHROW(cache.erase("missing"));
    }
}

TEST_CASE("Hash Cache - capacity and eviction", "[hash]") {
    hash_cache cache(2);
    
    SECTION("respects capacity limit") {
        std::vector<std::byte> d1(10, std::byte(0x01));
        std::vector<std::byte> d2(10, std::byte(0x02));
        std::vector<std::byte> d3(10, std::byte(0x03));
        
        cache.hash_file("f1", d1);
        cache.hash_file("f2", d2);
        cache.hash_file("f3", d3);
        
        REQUIRE(!cache.has_file("f1"));
        REQUIRE(cache.has_file("f2"));
        REQUIRE(cache.has_file("f3"));
    }
}

TEST_CASE("Hash Cache - large files", "[hash]") {
    hash_cache cache(3);
    
    SECTION("handles large binary data") {
        std::vector<std::byte> largeData(10000);
        for(size_t i = 0; i < largeData.size(); ++i) {
            largeData[i] = std::byte(i % 256);
        }
        
        std::string hash = cache.hash_file("large", largeData);
        REQUIRE(!hash.empty());
        REQUIRE(cache.has_file("large"));
    }
}
