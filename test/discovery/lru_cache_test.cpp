#include <catch2/catch_test_macros.hpp>
#include "lru_cache.h"

TEST_CASE("LRU cache basic operations", "[lru]") {
    lru_cache<int> cache(3);

    SECTION("put and get") {
        cache.put("a", 1);
        REQUIRE(cache.get("a") == 1);
        REQUIRE(cache.contains("a"));
    }

    SECTION("eviction policy") {
        cache.put("a", 1);
        cache.put("b", 2);
        cache.put("c", 3);
        cache.put("d", 4); // evicts "a" (LRU)
        REQUIRE_FALSE(cache.contains("a"));
        REQUIRE(cache.get("b") == 2);
        REQUIRE(cache.get("c") == 3);
        REQUIRE(cache.get("d") == 4);
    }

    SECTION("recency update") {
        cache.put("a", 1);
        cache.put("b", 2);
        cache.put("c", 3);
        cache.get("a");          // "a" becomes most recent
        cache.put("d", 4);       // should evict "b" (least recent now)
        REQUIRE_FALSE(cache.contains("b"));
        REQUIRE(cache.contains("a"));
        REQUIRE(cache.get("a") == 1);
    }

    SECTION("erase") {
        cache.put("x", 100);
        REQUIRE(cache.contains("x"));
        cache.erase("x");
        REQUIRE_FALSE(cache.contains("x"));
        REQUIRE_THROWS_AS(cache.get("x"), std::out_of_range);
    }

    SECTION("clear") {
        cache.put("a", 1);
        cache.put("b", 2);
        cache.clear();
        REQUIRE_FALSE(cache.contains("a"));
        REQUIRE_FALSE(cache.contains("b"));
    }
}
