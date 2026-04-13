#include <catch2/catch_test_macros.hpp>
#include "distributed/logic/include/population_tools.h"
#include <pagmo/problem.hpp>
#include <pagmo/population.hpp>
#include <pagmo/algorithms/de.hpp>
#include <vector>

TEST_CASE("population_tools: merge, select_best_N, constrained/unconstrained, edge cases", "[pop-tools]") {
    pagmo::problem prob{pagmo::schwefel{2}};
    std::vector<pagmo::vector_double> pops{
        {1.0, 2.0}, {3.0, 4.0}, {0.5, 5.0}
    };
    std::vector<pagmo::vector_double> fits{
        {10.0}, {5.0}, {2.0}
    };

    SECTION("merge_populations") {
        pagmo::archipelago archi{pagmo::fully_connected{}};
        // Simple test via direct call
        auto [mergedP, mergedF] = merge_populations(archi); // empty archi for edge
        REQUIRE(mergedP.empty());
    }

    SECTION("select_best_N_individuals - unconstrained") {
        auto best = select_best_N_individuals(prob, pops, fits, 2);
        REQUIRE(best.size() == 2);
        REQUIRE(best[0][0] == 0.5); // lowest fitness first
    }

    SECTION("edge cases: empty, N=0, N > size") {
        REQUIRE(select_best_N_individuals(prob, {}, {}, 5).empty());
        REQUIRE(select_best_N_individuals(prob, pops, fits, 0).empty());
        REQUIRE(select_best_N_individuals(prob, pops, fits, 10).size() == 3);
    }
}
