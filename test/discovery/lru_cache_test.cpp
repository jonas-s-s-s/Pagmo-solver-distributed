#include <catch2/catch_test_macros.hpp>
#include "distributed/discovery/include/lru_cache.h"
#include <string>

TEST_CASE("lru_cache: put, get, contains, eviction, erase, clear, exceptions", "[lru]") {
    lru_cache<std::string> cache(3);

    SECTION("basic put and get") {
        cache.put("key1", "val1");
        REQUIRE(cache.contains("key1"));
        REQUIRE(cache.get("key1") == "val1");
    }

    SECTION("eviction policy") {
        cache.put("a", "1");
        cache.put("b", "2");
        cache.put("c", "3");
        cache.put("d", "4");
        REQUIRE_FALSE(cache.contains("a"));
        REQUIRE(cache.contains("b"));
        REQUIRE(cache.contains("c"));
        REQUIRE(cache.contains("d"));
    }

    SECTION("recency update on get") {
        cache.put("x", "10");
        cache.put("y", "20");
        cache.put("z", "30");
        cache.get("x");
        cache.put("w", "40");
        REQUIRE_FALSE(cache.contains("y"));
        REQUIRE(cache.contains("x"));
        REQUIRE(cache.contains("z"));
        REQUIRE(cache.contains("w"));
    }

    SECTION("erase and clear") {
        cache.put("p", "100");
        cache.put("q", "200");
        cache.erase("p");
        REQUIRE_FALSE(cache.contains("p"));
        REQUIRE(cache.contains("q"));
        cache.clear();
        REQUIRE_FALSE(cache.contains("q"));
    }

    SECTION("exception on missing key") {
        REQUIRE_THROWS_AS(cache.get("missing"), std::out_of_range);
    }

    SECTION("update existing key") {
        cache.put("u", "old");
        cache.put("u", "new");
        REQUIRE(cache.get("u") == "new");
    }
}
