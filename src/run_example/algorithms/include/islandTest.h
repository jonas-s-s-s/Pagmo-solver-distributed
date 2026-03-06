#pragma once
#include "pagmo/algorithm.hpp"
#include "pagmo/problem.hpp"

namespace islandTest
{
    inline constexpr int GEN_COUNT = 1500;
    inline constexpr int POPULATION_SIZE = 100;

    /*
     * Utility function which runs an algorithm on some problem for GEN_COUNT generations with POPULATION_SIZE
     */
    void run_algorithm_on_problem(const pagmo::problem& problem, const std::vector<pagmo::algorithm>& algorithm);
    
    void run_zdt(const std::vector<pagmo::algorithm>& algorithm);

    void run_cec2014(const std::vector<pagmo::algorithm>& algorithm);

    /*
     * Non-dominated Sorting GA (NSGA2) pagmo::nsga2 M-U-I
     */
    void run_nsga2(const std::function<void(const std::vector<pagmo::algorithm>&)>& problemRunner);

    void run_de(const std::function<void(const std::vector<pagmo::algorithm>&)>& problemRunner);

    void run_meta_multiobjective(const std::function<void(const std::vector<pagmo::algorithm>&)>& problemRunner);

}
