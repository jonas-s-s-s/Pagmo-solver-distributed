#include <catch2/catch_test_macros.hpp>
#include "lib_loader.h"
#include <filesystem>

// Minimal test: only error paths (no actual DLL loading)
TEST_CASE("lib_loader error handling", "[lib-loader]") {
    SECTION("open_lib throws on nonexistent file") {
        lib_loader<void> loader("./does_not_exist.dll");
        REQUIRE_THROWS_AS(loader.open_lib(), std::runtime_error);
    }

    SECTION("get_instance throws if library not opened") {
        lib_loader<void> loader("./nonexistent.dll");
        REQUIRE_THROWS_AS(loader.get_instance(), std::runtime_error);
    }

    SECTION("close_lib does nothing if handle is null") {
        lib_loader<void> loader("./whatever.dll");
        REQUIRE_NOTHROW(loader.close_lib());
    }
}
