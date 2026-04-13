#include <catch2/catch_test_macros.hpp>
#include <string>
#include "distributed/discovery/include/lru_cache.h"

TEST_CASE("lru_cache basic put/get/contains", "[lru]") {
    lru_cache<std::string> cache(3);
    cache.put("key1", "val1");
    REQUIRE(cache.contains("key1"));
    REQUIRE(cache.get("key1") == "val1");
    cache.put("key2", "val2");
    REQUIRE(cache.contains("key2"));
}

TEST_CASE("lru_cache eviction", "[lru]") {
    lru_cache<int> cache(2);
    cache.put("a", 1);
    cache.put("b", 2);
    cache.put("c", 3); // evicts 'a'
    REQUIRE_FALSE(cache.contains("a"));
    REQUIRE(cache.contains("b"));
    REQUIRE(cache.contains("c"));
}

TEST_CASE("lru_cache recency and erase/clear", "[lru]") {
    lru_cache<int> cache(3);
    cache.put("a", 1);
    cache.put("b", 2);
    cache.get("a"); // update recency
    cache.put("c", 3);
    cache.erase("b");
    REQUIRE(cache.contains("a"));
    REQUIRE_FALSE(cache.contains("b"));
    cache.clear();
    REQUIRE_FALSE(cache.contains("a"));
}

TEST_CASE("lru_cache throws on missing key", "[lru]") {
    lru_cache<int> cache(1);
    REQUIRE_THROWS_AS(cache.get("missing"), std::out_of_range);
}
