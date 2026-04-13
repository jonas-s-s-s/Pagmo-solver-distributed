#include <catch2/catch_test_macros.hpp>
#include "population_tools.h"
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/archipelago.hpp>
#include <pagmo/islands/thread_island.hpp>
#include <pagmo/algorithms/de.hpp>

TEST_CASE("Population tools", "[pop-tools]") {
    pagmo::problem prob_sp{pagmo::rosenbrock{2}};
    pagmo::problem prob_mo{pagmo::zdt{1, 2}};

    SECTION("merge_populations") {
        pagmo::archipelago archi;
        for (int i = 0; i < 3; ++i) {
            pagmo::population pop{prob_sp, 5};
            archi.push_back(pagmo::island{pagmo::algorithm{pagmo::de{100}}, pop});
        }
        auto [pops, fits] = merge_populations(archi);
        REQUIRE(pops.size() == 15);
        REQUIRE(fits.size() == 15);
    }

    SECTION("select_best_N_individuals - unconstrained") {
        std::vector<pagmo::vector_double> pops = {{1,2}, {3,4}, {0,1}, {2,3}};
        std::vector<pagmo::vector_double> fits = {{10}, {5}, {1}, {8}}; // lower is better
        auto best = select_best_N_individuals(prob_sp, pops, fits, 2);
        REQUIRE(best.size() == 2);
        REQUIRE(best[0] == pagmo::vector_double{0,1}); // fitness 1
        REQUIRE(best[1] == pagmo::vector_double{3,4}); // fitness 5
    }

    SECTION("select_best_N_individuals - empty input") {
        std::vector<pagmo::vector_double> empty;
        auto best = select_best_N_individuals(prob_sp, empty, empty, 5);
        REQUIRE(best.empty());
    }

    SECTION("N = 0 returns empty") {
        std::vector<pagmo::vector_double> pops = {{1,2}};
        std::vector<pagmo::vector_double> fits = {{10}};
        auto best = select_best_N_individuals(prob_sp, pops, fits, 0);
        REQUIRE(best.empty());
    }

    SECTION("select_best_N_into_new_population") {
        std::vector<pagmo::vector_double> pops = {{1,2}, {3,4}, {0,1}, {2,3}};
        std::vector<pagmo::vector_double> fits = {{10}, {5}, {1}, {8}};
        pagmo::population new_pop = select_best_N_into_new_population(prob_sp, pops, fits, 2);
        REQUIRE(new_pop.size() == 2);
        // The first individual should be the best (0,1)
        REQUIRE(new_pop.get_x()[0] == pagmo::vector_double{0,1});
    }
}
