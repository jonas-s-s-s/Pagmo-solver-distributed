#include <catch2/catch_test_macros.hpp>
#include "lru_cache.h"

TEST_CASE("LRU Cache - put operation", "[lru]") {
    lru_cache<std::string> cache(3);
    
    SECTION("put single item") {
        cache.put("key1", "value1");
        REQUIRE(cache.contains("key1"));
    }
    
    SECTION("put multiple items") {
        cache.put("key1", "value1");
        cache.put("key2", "value2");
        cache.put("key3", "value3");
        
        REQUIRE(cache.contains("key1"));
        REQUIRE(cache.contains("key2"));
        REQUIRE(cache.contains("key3"));
    }
    
    SECTION("put overwrites existing value") {
        cache.put("key1", "value1");
        cache.put("key1", "updated");
        REQUIRE(cache.get("key1") == "updated");
    }
}

TEST_CASE("LRU Cache - get operation", "[lru]") {
    lru_cache<std::string> cache(3);
    
    SECTION("get returns correct value") {
        cache.put("key1", "value1");
        REQUIRE(cache.get("key1") == "value1");
    }
    
    SECTION("get updates recency") {
        cache.put("key1", "val1");
        cache.put("key2", "val2");
        cache.put("key3", "val3");
        
        cache.get("key1");
        cache.put("key4", "val4");
        
        REQUIRE(cache.contains("key1"));
        REQUIRE(!cache.contains("key2"));
    }
    
    SECTION("get throws for missing key") {
        REQUIRE_THROWS_AS(cache.get("missing"), std::out_of_range);
    }
}

TEST_CASE("LRU Cache - contains operation", "[lru]") {
    lru_cache<std::string> cache(2);
    
    SECTION("contains true for present key") {
        cache.put("key1", "val");
        REQUIRE(cache.contains("key1"));
    }
    
    SECTION("contains false for absent key") {
        REQUIRE(!cache.contains("missing"));
    }
    
    SECTION("contains false for evicted key") {
        cache.put("key1", "val1");
        cache.put("key2", "val2");
        cache.put("key3", "val3");
        
        REQUIRE(!cache.contains("key1"));
    }
}

TEST_CASE("LRU Cache - eviction policy", "[lru]") {
    lru_cache<std::string> cache(2);
    
    SECTION("least recently used is evicted") {
        cache.put("key1", "val1");
        cache.put("key2", "val2");
        cache.put("key3", "val3");
        
        REQUIRE(!cache.contains("key1"));
        REQUIRE(cache.contains("key2"));
        REQUIRE(cache.contains("key3"));
    }
    
    SECTION("access updates recency order") {
        cache.put("key1", "val1");
        cache.put("key2", "val2");
        cache.get("key1");
        cache.put("key3", "val3");
        
        REQUIRE(cache.contains("key1"));
        REQUIRE(!cache.contains("key2"));
        REQUIRE(cache.contains("key3"));
    }
    
    SECTION("put update moves to front") {
        cache.put("key1", "val1");
        cache.put("key2", "val2");
        cache.put("key1", "updated");
        cache.put("key3", "val3");
        
        REQUIRE(cache.contains("key1"));
        REQUIRE(!cache.contains("key2"));
    }
}

TEST_CASE("LRU Cache - erase operation", "[lru]") {
    lru_cache<std::string> cache(3);
    
    SECTION("erase removes item") {
        cache.put("key1", "val");
        cache.erase("key1");
        REQUIRE(!cache.contains("key1"));
    }
    
    SECTION("erase non-existent is safe") {
        REQUIRE_NOTHROW(cache.erase("missing"));
    }
    
    SECTION("erase can create space") {
        cache.put("key1", "val1");
        cache.put("key2", "val2");
        cache.put("key3", "val3");
        cache.erase("key1");
        cache.put("key4", "val4");
        
        REQUIRE(!cache.contains("key1"));
        REQUIRE(cache.contains("key4"));
    }
}

TEST_CASE("LRU Cache - clear operation", "[lru]") {
    lru_cache<std::string> cache(3);
    
    cache.put("key1", "val1");
    cache.put("key2", "val2");
    cache.put("key3", "val3");
    
    cache.clear();
    
    REQUIRE(!cache.contains("key1"));
    REQUIRE(!cache.contains("key2"));
    REQUIRE(!cache.contains("key3"));
}

TEST_CASE("LRU Cache - edge cases", "[lru]") {
    SECTION("capacity of 1") {
        lru_cache<std::string> cache(1);
        cache.put("k1", "v1");
        cache.put("k2", "v2");
        
        REQUIRE(!cache.contains("k1"));
        REQUIRE(cache.contains("k2"));
    }
    
    SECTION("capacity of 0 creates minimal behavior") {
        lru_cache<int> cache(1);
        cache.put("x", 10);
        REQUIRE(cache.get("x") == 10);
    }
}
