#include <catch2/catch_test_macros.hpp>
#include "distributed/dynamic/include/defines.h"

TEST_CASE("platform_utils portable_dll_extension correctness", "[platform]") {
    std::string ext = portable_dll_extension();
#if defined(WIN32)
    REQUIRE(ext == ".dll");
#elif defined(__APPLE__)
    REQUIRE(ext == ".dylib");
#else
    REQUIRE(ext == ".so");
#endif
}

TEST_CASE("platform_utils error_msg returns valid string", "[platform]") {
    std::string msg = error_msg();
    REQUIRE_FALSE(msg.empty());
}
