#include "islandTest.h"

#include "distributed_controller.h"
#include "distributed_island.h"
#include "pagmo/algorithms/pso.hpp"
#include "pagmo/algorithm.hpp"
#include "pagmo/archipelago.hpp"
#include "pagmo/problem.hpp"
#include "pagmo/algorithms/nsga2.hpp"
#include "pagmo/algorithms/xnes.hpp"
#include "pagmo/problems/zdt.hpp"
#include "pagmo/utils/multi_objective.hpp"

void islandTest::run_algorithm_on_problem(const pagmo::problem& problem, const pagmo::algorithm& algorithm)
{
    pagmo::archipelago archi{};
    // Construct 8 distributed islands and add them to archipelago
    for (int i = 0; i < 1; ++i)
    {
        pagmo::distributed_island dist_island{};
        archi.push_back(pagmo::island{dist_island, algorithm, problem, POPULATION_SIZE});
    }

    // Run evolution 2 times on each island of this archi
    archi.evolve(1);
    // Wait for evolution to finish
    archi.wait_check();


    for (int i = 0; i < archi.size(); ++i)
    {
        const auto& isl = archi[i];

        // Output the population
        std::cout << "Island #" << i + 1 << std::endl;
        std::cout << "Best 5 individual:" << std::endl;
        const auto pop_decision_vectors = isl.get_population().get_x();
        const auto sorted_indexes = pagmo::sort_population_mo(pop_decision_vectors);
        for (int i = 0; i < 5 && i < sorted_indexes.size(); ++i)
        {
            const auto individual = pop_decision_vectors.at(sorted_indexes.at(i));
            std::cout << "[ ";
            for (const double val : individual)
            {
                std::cout << val << ", ";
            }
            std::cout << "]" << std::endl;
        }

        std::cout << std::endl;
    }
}

void islandTest::run_zdt(const pagmo::algorithm& algorithm)
{
    std::cout << std::endl << "Running ZDT for algorithm: " << algorithm.get_name() << std::endl;
    std::cout << "==================================================================" << std::endl << std::endl;
    for (int i = 1; i <= 1; ++i) //6
    {
        //std::cout << "-------------------------------------------------------" << std::endl;
        //std::cout << "ZDT problem No." << i << std::endl;
        const pagmo::problem prob{pagmo::zdt(i)};
        run_algorithm_on_problem(prob, algorithm);
    }
}

void islandTest::run_nsga2(const std::function<void(const pagmo::algorithm&)>& problemRunner)
{
    pagmo::algorithm nsga2{pagmo::nsga2(GEN_COUNT)};
    nsga2.set_verbosity(0);
    problemRunner(nsga2);
}
