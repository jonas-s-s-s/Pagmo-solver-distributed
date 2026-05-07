#include <catch2/catch_test_macros.hpp>
#include "population_tools.h"
#include "pagmo/problems/schwefel.hpp"

#include "pagmo/algorithms/de.hpp"

TEST_CASE("Population Tools - merge_populations", "[pop-tools]")
{
    pagmo::problem prob(pagmo::schwefel(3));
    
    SECTION("single island") {
        pagmo::archipelago archi;
        archi.push_back(pagmo::island(pagmo::de(100), prob, 10));
        
        auto [pops, fits] = merge_populations(archi);
        REQUIRE(pops.size() == 10);
        REQUIRE(fits.size() == 10);
    }
    
    SECTION("multiple islands") {
        pagmo::archipelago archi;
        archi.push_back(pagmo::island(pagmo::de(100), prob, 5));
        archi.push_back(pagmo::island(pagmo::de(100), prob, 7));
        archi.push_back(pagmo::island(pagmo::de(100), prob, 3));
        
        auto [pops, fits] = merge_populations(archi);
        REQUIRE(pops.size() >= 15);
        REQUIRE(fits.size() >= 15);
    }
}

TEST_CASE("Population Tools - select_best_N_individuals", "[pop-tools]") {
    pagmo::problem prob(pagmo::schwefel(3));
    pagmo::archipelago archi;
    archi.push_back(pagmo::island(pagmo::de(100), prob, 30));
    
    auto [pops, fits] = merge_populations(archi);
    
    SECTION("select N < size") {
        auto best = select_best_N_individuals(prob, pops, fits, 10);
        REQUIRE(best.size() == 10);
    }
    
    SECTION("select N = size") {
        auto best = select_best_N_individuals(prob, pops, fits, pops.size());
        REQUIRE(best.size() == pops.size());
    }
    
    SECTION("select N = 0") {
        auto best = select_best_N_individuals(prob, pops, fits, 0);
        REQUIRE(best.size() == 0);
    }

}

TEST_CASE("Population Tools - select_best_N_into_new_population", "[pop-tools]") {
    pagmo::problem prob(pagmo::schwefel(3));
    pagmo::archipelago archi;
    archi.push_back(pagmo::island(pagmo::de(100), prob, 20));
    
    auto [pops, fits] = merge_populations(archi);
    
    SECTION("create new population") {
        auto newPop = select_best_N_into_new_population(prob, pops, fits, 5);
        REQUIRE(newPop.size() == 5);
    }
    
    SECTION("population contains valid individuals") {
        auto newPop = select_best_N_into_new_population(prob, pops, fits, 5);
        auto x = newPop.get_x();
        REQUIRE(x.size() == 5);
    }
}

TEST_CASE("Population Tools - select_best_individual", "[pop-tools]") {
    pagmo::problem prob(pagmo::schwefel(3));
    pagmo::archipelago archi;
    archi.push_back(pagmo::island(pagmo::de(100), prob, 20));
    
    auto [pops, fits] = merge_populations(archi);
    
    SECTION("returns single best") {
        auto best = select_best_individual(prob, pops, fits);
        REQUIRE(best.size() == 3);
    }
}

TEST_CASE("Population Tools - constrained vs unconstrained", "[pop-tools]") {
    pagmo::problem prob_uc(pagmo::schwefel(3));
    pagmo::archipelago archi;
    archi.push_back(pagmo::island(pagmo::de(100), prob_uc, 20));
    
    auto [pops, fits] = merge_populations(archi);
    auto best = select_best_N_individuals(prob_uc, pops, fits, 5);
    
    REQUIRE(best.size() == 5);
}
