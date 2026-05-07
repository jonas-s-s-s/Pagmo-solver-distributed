#include <catch2/catch_test_macros.hpp>
#include "distributed_solver.h"
#include <chrono>

TEST_CASE("System initialization and basic operations", "[integration]") {
    SECTION("initial state is idle") {
        distributed_solver ds("tcp://localhost:8001", 1);
        REQUIRE(ds.get_status() == pagmo::evolve_status::idle);
    }
    
    SECTION("worker count accessible") {
        distributed_solver ds("tcp://localhost:8002", 1);
        REQUIRE(ds.get_current_worker_count() == 0);
    }
    
    SECTION("expected worker count") {
        distributed_solver ds("tcp://localhost:8003", 5);
        REQUIRE(ds.get_expected_worker_count() == 5);
    }
}

