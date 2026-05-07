#include <catch2/catch_test_macros.hpp>
#include "distributed_solver.h"

TEST_CASE("Distributed Solver - constructor", "[work-plan]") {
    REQUIRE_NOTHROW(distributed_solver ds("tcp://localhost:5555", 1));
}

TEST_CASE("Distributed Solver - get expected worker count", "[work-plan]") {
    distributed_solver ds("tcp://localhost:5556", 4);
    REQUIRE(ds.get_expected_worker_count() == 4);
}
