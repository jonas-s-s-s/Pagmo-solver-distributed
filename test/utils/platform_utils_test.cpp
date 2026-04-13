#include <catch2/catch_test_macros.hpp>
#include "distributed/dynamic/include/defines.h"
#include <string>

TEST_CASE("platform_utils: portable_dll_extension and error_msg", "[platform]") {
    SECTION("portable_dll_extension returns correct extension") {
        std::string ext = portable_dll_extension();
#if defined(WIN32)
        REQUIRE(ext == ".dll");
#elif defined(__APPLE__)
        REQUIRE(ext == ".dylib");
#else
        REQUIRE(ext == ".so");
#endif
    }

    SECTION("error_msg returns valid non-empty string") {
        std::string msg = error_msg();
        REQUIRE_FALSE(msg.empty());
    }
}
