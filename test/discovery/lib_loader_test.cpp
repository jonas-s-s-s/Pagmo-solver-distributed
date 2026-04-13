#include <catch2/catch_test_macros.hpp>
#include "distributed/dynamic/include/lib_loader.h"
#include <filesystem>
#include <string>

TEST_CASE("lib_loader: error handling paths and basic behavior (minimal)", "[lib-loader]") {
    SECTION("open_lib throws on non-existent file") {
        lib_loader<void> loader{"/non/existent/path/that/does/not/exist.so"};
        REQUIRE_THROWS_AS(loader.open_lib(), std::runtime_error);
    }

    SECTION("close_lib on null handle does nothing") {
        lib_loader<void> loader{"/non/existent/path/that/does/not/exist.so"};
        REQUIRE_NOTHROW(loader.close_lib());  // Should be safe
    }

    SECTION("get_instance / clone_instance throw when lib not opened") {
        lib_loader<void> loader{"/non/existent/path/that/does/not/exist.so"};
        REQUIRE_THROWS_AS(loader.get_instance(), std::runtime_error);
        REQUIRE_THROWS_AS(loader.clone_instance(nullptr), std::runtime_error);
    }

    SECTION("portable symbols are correctly resolved in error path") {
        // Minimal check that error_msg produces non-empty string
        std::string msg = error_msg();
        REQUIRE_FALSE(msg.empty());
    }
}
