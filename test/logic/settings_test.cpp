#include <catch2/catch_test_macros.hpp>
#include "distributed/logic/include/settings.h"
#include "distributed/logic/include/worker_settings.h"
#include <filesystem>
#include <fstream>

TEST_CASE("settings: missing file init, save/reload, temporary file", "[settings]") {
    const std::string tmpFile = "test_settings_tmp.xml";

    SECTION("missing file triggers initialize()") {
        if (std::filesystem::exists(tmpFile)) std::filesystem::remove(tmpFile);
        settings<worker_settings> s{tmpFile, false};
        REQUIRE_FALSE(s().workerId.empty());
    }

    SECTION("save and reload") {
        worker_settings ws;
        ws.workerId = "test_worker_abc123";
        {
            settings<worker_settings> s{tmpFile, false};
            s().workerId = ws.workerId;
            s.save();
        }
        settings<worker_settings> s2{tmpFile, false};
        REQUIRE(s2().workerId == "test_worker_abc123");
        std::filesystem::remove(tmpFile);
    }
}
