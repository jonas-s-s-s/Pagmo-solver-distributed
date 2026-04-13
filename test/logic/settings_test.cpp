#include <catch2/catch_test_macros.hpp>
#include "settings.h"
#include "worker_settings.h"
#include <filesystem>
#include <fstream>

TEST_CASE("Settings file handling", "[settings]") {
    std::filesystem::path temp_file = std::filesystem::temp_directory_path() / "test_settings.xml";
    std::filesystem::remove(temp_file); // ensure clean

    SECTION("missing file triggers initialization") {
        REQUIRE_FALSE(std::filesystem::exists(temp_file));
        settings<worker_settings> s(temp_file);
        auto& ws = s();
        REQUIRE_FALSE(ws.workerId.empty());
    }

    SECTION("save and reload settings") {
        settings<worker_settings> s(temp_file);
        std::string original_id = s().workerId;
        s.save();

        settings<worker_settings> s2(temp_file);
        REQUIRE(s2().workerId == original_id);
    }

    // Cleanup
    std::filesystem::remove(temp_file);
}
