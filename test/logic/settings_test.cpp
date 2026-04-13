#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include "distributed/logic/include/settings.h"
#include "distributed/logic/include/worker_settings.h"

TEST_CASE("settings missing file triggers initialize and save/reload", "[settings]") {
    std::filesystem::path tmp = "test_settings_tmp.xml";
    if (std::filesystem::exists(tmp)) std::filesystem::remove(tmp);

    {
        settings<worker_settings> s(tmp, false);
        REQUIRE(s().workerId.find("worker_") == 0);
    }

    REQUIRE(std::filesystem::exists(tmp));
    std::ifstream f(tmp);
    REQUIRE(f.good());

    std::filesystem::remove(tmp);
}
