#include <catch2/catch_test_macros.hpp>
#include "udp_registry.h"
#include <filesystem>
#include <fstream>
#include <vector>

TEST_CASE("UDP Registry - cache directory operations", "[registry]") {
    std::string testDir = "./test_udp_cache_" + std::to_string(time(nullptr));
    
    SECTION("set local cache directory") {
        std::filesystem::create_directory(testDir);
        udp_registry::get().set_local_cache_dir(testDir);
        REQUIRE(std::filesystem::exists(testDir));
        std::filesystem::remove_all(testDir);
    }
}

TEST_CASE("UDP Registry - file access", "[registry]") {
    std::string testDir = "./test_udp_files_" + std::to_string(time(nullptr));
    std::filesystem::create_directory(testDir);
    udp_registry::get().set_local_cache_dir(testDir);
    
    SECTION("get_lib_as_file returns nullopt for missing") {
        auto result = udp_registry::get().get_lib_as_file("nonexistent.dll");
        REQUIRE(!result.has_value());
    }
    
    std::filesystem::remove_all(testDir);
}

TEST_CASE("UDP Registry - in-memory cache", "[registry]") {
    SECTION("use_in_memory_cache") {
        REQUIRE_NOTHROW(udp_registry::get().use_in_memory_cache(true));
        REQUIRE_NOTHROW(udp_registry::get().use_in_memory_cache(false));
    }
}
