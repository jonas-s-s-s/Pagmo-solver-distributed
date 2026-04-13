#include <catch2/catch_test_macros.hpp>
#include "defines.h"

TEST_CASE("Platform utilities", "[utils]") {
    SECTION("portable_dll_extension") {
        auto ext = portable_dll_extension();
        
        REQUIRE(!ext.empty());
        REQUIRE(ext[0] == '.');
        
        #if defined(WIN32)
            REQUIRE(ext == ".dll");
        #elif defined(__APPLE__)
            REQUIRE(ext == ".dylib");
        #elif defined(__linux__)
            REQUIRE(ext == ".so");
        #endif
    }
    
    SECTION("error_msg non-empty") {
        auto msg = error_msg();
        REQUIRE(!msg.empty());
    }
}
