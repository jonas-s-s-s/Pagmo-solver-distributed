#include <catch2/catch_test_macros.hpp>
#include "worker_info_repository.h"
#include <thread>

TEST_CASE("Worker info repository", "[worker-info]") {
    worker_info_repository repo;

    SECTION("worker join and leave") {
        repo.worker_joined("w1");
        REQUIRE(repo.get_worker_count() == 1);
        auto connected = repo.get_connected_workers();
        REQUIRE(connected.find("w1") != connected.end());

        repo.worker_left("w1");
        REQUIRE(repo.get_worker_count() == 0);
        REQUIRE(repo.get_connected_workers().empty());
    }

    SECTION("work start/finish stats") {
        repo.worker_joined("w2");
        repo.worker_started_work("w2");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        repo.worker_finished_work("w2", 100, "DE");

        auto info = repo.get_worker_info("w2");
        REQUIRE(info.has_value());
        REQUIRE(info->totalStats.processedPopulation == 100);
        REQUIRE(info->totalStats.workTime >= 10);
        REQUIRE(info->statsByAlgorithm.count("DE") == 1);
        REQUIRE(info->statsByAlgorithm.at("DE").processedPopulation == 100);
    }

    SECTION("query unknown worker") {
        auto info = repo.get_worker_info("unknown");
        REQUIRE_FALSE(info.has_value());
    }

    SECTION("worker_joined with custom info") {
        worker_info custom;
        custom.totalStats.processedPopulation = 999;
        repo.worker_joined("w3", custom);
        auto info = repo.get_worker_info("w3");
        REQUIRE(info.has_value());
        REQUIRE(info->totalStats.processedPopulation == 999);
    }
}
