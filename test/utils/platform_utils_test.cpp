#include <catch2/catch_test_macros.hpp>
#include "defines.h"

TEST_CASE("Platform utilities", "[platform]") {
    SECTION("portable_dll_extension") {
        std::string ext = portable_dll_extension();
#if defined(WIN32)
        REQUIRE(ext == ".dll");
#elif defined(__APPLE__)
        REQUIRE(ext == ".dylib");
#elif defined(__linux__)
        REQUIRE(ext == ".so");
#else
        FAIL("Unknown platform");
#endif
    }

    SECTION("error_msg returns non-empty string") {
        std::string err = error_msg();
        // On Windows it returns a message like "LoadLibrary / CloseLibrary error code: ..."
        // On Unix it may be empty if no error occurred, but the function always returns something.
        REQUIRE_FALSE(err.empty());
    }
}
