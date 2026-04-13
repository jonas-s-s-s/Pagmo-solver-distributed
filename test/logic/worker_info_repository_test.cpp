#include <catch2/catch_test_macros.hpp>
#include "distributed/logic/include/worker_info_repository.h"
#include <chrono>
#include <thread>

TEST_CASE("worker_info_repository join/leave/tracking", "[worker-info]") {
    worker_info_repository repo;
    repo.worker_joined("w1");
    REQUIRE(repo.get_worker_count() == 1);
    REQUIRE(repo.get_connected_workers().contains("w1"));

    repo.worker_left("w1");
    REQUIRE(repo.get_worker_count() == 0);

    repo.worker_joined("w2");
    repo.worker_started_work("w2");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    repo.worker_finished_work("w2", 100, "de");
    auto info = repo.get_worker_info("w2");
    REQUIRE(info.has_value());
    REQUIRE(info->totalStats.processedPopulation == 100);
}

TEST_CASE("worker_info_repository wait_until_worker_count", "[worker-info]") {
    worker_info_repository repo;
    std::thread t([&repo]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        repo.worker_joined("w3");
    });
    repo.wait_until_worker_count(1);
    REQUIRE(repo.get_worker_count() == 1);
    t.join();
}
