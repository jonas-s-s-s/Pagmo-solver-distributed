#include <catch2/catch_test_macros.hpp>
#include "distributed_solver.h"
#include "pagmo/algorithms/de.hpp"
#include "pagmo/problems/schwefel.hpp"

TEST_CASE("Work plan generation and distribution", "[work-plan]")
{
    SECTION("equal distribution strategy") {
        distributed_solver ds("tcp://localhost:7000", 4, load_balancing_strategy::ALL_ISLANDS_EQUAL);
        REQUIRE(ds.get_expected_worker_count() == 4);
    }
    
    SECTION("performance-based strategy") {
        distributed_solver ds("tcp://localhost:7001", 2, load_balancing_strategy::BY_PERFORMANCE);
        REQUIRE(ds.get_expected_worker_count() == 2);
    }
    
    SECTION("population size constraints") {
        distributed_solver ds("tcp://localhost:7002", 1);
        pagmo::problem prob(pagmo::schwefel(5));

        REQUIRE_NOTHROW(ds.get_expected_worker_count());
    }
}

TEST_CASE("Solver initialization", "[work-plan]") {
    SECTION("construct with different parameters") {
        REQUIRE_NOTHROW(distributed_solver("tcp://localhost:7003", 1));
        REQUIRE_NOTHROW(distributed_solver("tcp://localhost:7004", 4, load_balancing_strategy::ALL_ISLANDS_EQUAL));
        REQUIRE_NOTHROW(distributed_solver("tcp://localhost:7005", 2, load_balancing_strategy::BY_PERFORMANCE));
    }
    
    SECTION("initial state") {
        distributed_solver ds("tcp://localhost:7006", 1);
        REQUIRE(ds.get_status() == pagmo::evolve_status::idle);
    }
}
