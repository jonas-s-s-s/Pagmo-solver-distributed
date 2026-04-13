#include <catch2/catch_test_macros.hpp>
#include "settings.h"
#include "worker_settings.h"
#include <filesystem>
#include <fstream>

TEST_CASE("Settings - basic operations", "[settings]") {
    std::string tempFile = "test_settings_" + std::to_string(time(nullptr)) + ".xml";
    
    SECTION("save and load") {
        {
            settings<worker_settings> s(tempFile, false);
            s().workerId = "test_worker_id";
            s.save();
        }
        
        {
            settings<worker_settings> s(tempFile, false);
            REQUIRE(s().workerId == "test_worker_id");
        }
    }
    
    SECTION("missing file initializes") {
        std::string newFile = "new_settings_" + std::to_string(time(nullptr)) + ".xml";
        std::filesystem::remove(newFile);
        
        settings<worker_settings> s(newFile, false);
        REQUIRE(!s().workerId.empty());
        
        std::filesystem::remove(newFile);
    }
    
    SECTION("auto-save enabled") {
        std::string autoFile = "auto_settings_" + std::to_string(time(nullptr)) + ".xml";
        std::string savedId;
        
        {
            settings<worker_settings> s(autoFile, true);
            savedId = s().workerId;
            auto& ref = s();
        }
        
        {
            settings<worker_settings> s(autoFile, false);
            REQUIRE(s().workerId == savedId);
        }
        
        std::filesystem::remove(autoFile);
    }
    
    std::filesystem::remove(tempFile);
}

TEST_CASE("Settings - worker settings", "[settings]") {
    std::string tempFile = "worker_test_" + std::to_string(time(nullptr)) + ".xml";
    
    SECTION("worker id persists") {
        std::string originalId;
        {
            settings<worker_settings> s(tempFile, false);
            originalId = s().workerId;
            s.save();
        }
        
        {
            settings<worker_settings> s(tempFile, false);
            REQUIRE(s().workerId == originalId);
        }
    }
    
    std::filesystem::remove(tempFile);
}
