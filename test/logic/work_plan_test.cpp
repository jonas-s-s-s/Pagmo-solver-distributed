#include <catch2/catch_test_macros.hpp>
#include "distributed/logic/include/distributed_solver.h"
#include <pagmo/algorithms/de.hpp>
#include <pagmo/problems/schwefel.hpp>

TEST_CASE("work_plan equal distribution fallback when workers not ready", "[work-plan]") {
    distributed_solver ds{"ipc://test_plan_socket", 3, load_balancing_strategy::ALL_ISLANDS_EQUAL};
    pagmo::problem prob{pagmo::schwefel(3)};
    pagmo::algorithm algo{pagmo::de(10)};

    ds.evolve(prob, {algo, algo}, 120, 1, 20);
    REQUIRE(ds.get_status() != pagmo::evolve_status::busy);
}

TEST_CASE("work_plan performance-based when workers connected", "[work-plan]") {
    distributed_solver ds{"ipc://test_plan_socket", 2, load_balancing_strategy::BY_PERFORMANCE};

    pagmo::problem prob{pagmo::schwefel(3)};
    pagmo::algorithm algo{pagmo::de(10)};
    ds.evolve(prob, {algo}, 200, 1, 40);
    REQUIRE(ds.get_status() != pagmo::evolve_status::busy);
}
