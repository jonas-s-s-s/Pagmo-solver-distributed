#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <vector>
#include "distributed/discovery/include/udp_registry.h"

TEST_CASE("udp_registry cache lookup, temp dir storage and invalidation", "[registry]") {
    udp_registry& reg = udp_registry::get();
    std::filesystem::path tmpDir = std::filesystem::temp_directory_path() / "udp_registry_test";
    std::filesystem::create_directories(tmpDir);
    reg.set_local_cache_dir(tmpDir);
    reg.use_in_memory_cache(true);

    std::vector<std::byte> fakeLib{std::byte(0x01), std::byte(0x02)};
    reg.register_udp_provider([&](const std::string& name) -> std::optional<std::vector<std::byte>> {
        if (name == "test_lib") return fakeLib;
        return std::nullopt;
    });

    auto file1 = reg.get_lib_as_file("test_lib");
    REQUIRE(file1.has_value());
    REQUIRE(file1.value() == fakeLib);

    auto file2 = reg.get_lib_as_file("test_lib");
    REQUIRE(file2.has_value());

    std::filesystem::remove_all(tmpDir);
}

TEST_CASE("udp_registry error paths", "[registry]") {
    udp_registry& reg = udp_registry::get();
    REQUIRE_THROWS_AS(reg.construct_udp("nonexistent_lib"), std::runtime_error);
}
