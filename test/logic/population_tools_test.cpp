#include <catch2/catch_test_macros.hpp>
#include <pagmo/topologies/fully_connected.hpp>
#include "distributed/logic/include/population_tools.h"
#include <pagmo/problem.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problems/schwefel.hpp>
#include <pagmo/archipelago.hpp>

TEST_CASE("population_tools merge_populations and select_best_N", "[pop-tools]") {
    pagmo::problem prob{pagmo::schwefel(3)};
    pagmo::population p1{prob, 4};
    pagmo::population p2{prob, 4};
    pagmo::archipelago archi{pagmo::fully_connected{}};
    archi.push_back(pagmo::island{archi[0].get_algorithm(), p1});
    archi.push_back(pagmo::island{archi[0].get_algorithm(), p2});

    auto [allPops, allFits] = merge_populations(archi);
    REQUIRE(allPops.size() == 8);
    REQUIRE(allFits.size() == 8);

    auto best = select_best_N_individuals(prob, allPops, allFits, 2);
    REQUIRE(best.size() == 2);
}

TEST_CASE("population_tools edge cases empty / N=0 / N>size", "[pop-tools]") {
    pagmo::problem prob{pagmo::schwefel(3)};
    std::vector<pagmo::vector_double> emptyPop{};
    std::vector<pagmo::vector_double> emptyFit{};
    REQUIRE(select_best_N_individuals(prob, emptyPop, emptyFit, 5).empty());
    REQUIRE(select_best_N_individuals(prob, {{1.0}}, {{2.0}}, 0).empty());
}
