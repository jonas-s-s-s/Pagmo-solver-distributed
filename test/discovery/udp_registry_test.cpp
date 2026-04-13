#include <catch2/catch_test_macros.hpp>
#include "udp_registry.h"
#include <filesystem>
#include <fstream>

TEST_CASE("UDP registry cache and file operations", "[registry]") {
    // Use a temporary directory
    auto tmp_dir = std::filesystem::temp_directory_path() / "udp_reg_test";
    std::filesystem::remove_all(tmp_dir);
    std::filesystem::create_directory(tmp_dir);

    udp_registry& reg = udp_registry::get();
    reg.set_local_cache_dir(tmp_dir);
    reg.use_in_memory_cache(true);

    std::vector<std::byte> fake_dll(100, std::byte{0xAB});
    std::string lib_name = "test_lib";

    SECTION("get_lib_as_file – not present, with provider") {
        bool provider_called = false;
        reg.register_udp_provider([&](const std::string& name) -> std::optional<std::vector<std::byte>> {
            if (name == lib_name) {
                provider_called = true;
                return fake_dll;
            }
            return std::nullopt;
        });
        auto result = reg.get_lib_as_file(lib_name);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == fake_dll);
        REQUIRE(provider_called);
        // File should now be cached on disk
        auto disk_path = tmp_dir / (lib_name + portable_dll_extension());
        REQUIRE(std::filesystem::exists(disk_path));
    }

    SECTION("get_lib_file_hash – caches hash") {
        reg.get_lib_as_file(lib_name); // ensure file exists
        auto hash1 = reg.get_lib_file_hash(lib_name);
        REQUIRE(hash1.has_value());
        auto hash2 = reg.get_lib_file_hash(lib_name);
        REQUIRE(hash2 == hash1); // should be cached
    }

    SECTION("cache invalidation when file changes") {
        reg.get_lib_as_file(lib_name);
        auto old_hash = reg.get_lib_file_hash(lib_name);
        // Modify file on disk
        std::ofstream ofs(tmp_dir / (lib_name + portable_dll_extension()), std::ios::binary);
        ofs.write("different", 9);
        ofs.close();
        // New hash should be different (cache should be updated because file changed)
        auto new_hash = reg.get_lib_file_hash(lib_name);
        REQUIRE(new_hash != old_hash);
    }

    SECTION("initialize_udp throws on missing file") {
        REQUIRE_THROWS_AS(reg.initialize_udp("nonexistent", std::nullopt), std::runtime_error);
    }

    // Cleanup
    reg.use_in_memory_cache(false);
    std::filesystem::remove_all(tmp_dir);
}
