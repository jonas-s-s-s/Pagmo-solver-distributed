#include <catch2/catch_test_macros.hpp>
#include "distributed/logic/include/worker_info_repository.h"
#include <chrono>
#include <thread>

TEST_CASE("worker_info_repository: join/leave, tracking, stats, unknown worker", "[worker-info]") {
    worker_info_repository repo;

    SECTION("worker join/leave tracking") {
        repo.worker_joined("w1");
        REQUIRE(repo.get_worker_count() == 1);
        REQUIRE(repo.get_connected_workers().contains("w1"));
        repo.worker_left("w1");
        REQUIRE(repo.get_worker_count() == 0);
    }

    SECTION("work start/finish + stats accumulation") {
        repo.worker_joined("w2");
        repo.worker_started_work("w2");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        repo.worker_finished_work("w2", 100, "de");
        auto info = repo.get_worker_info("w2");
        REQUIRE(info.has_value());
        REQUIRE(info.value().totalStats.processedPopulation == 100);
        REQUIRE(info.value().statsByAlgorithm.contains("de"));
    }

    SECTION("querying unknown worker returns nullopt") {
        REQUIRE_FALSE(repo.get_worker_info("unknown").has_value());
    }
}
