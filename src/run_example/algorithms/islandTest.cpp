#include "islandTest.h"

#include "base_problem.h"
#include "distributed_controller.h"
#include "distributed_island.h"
#include "dll_problem_wrapper.h"
#include "lib_loader.h"
#include "pagmo/algorithms/pso.hpp"
#include "pagmo/algorithm.hpp"
#include "pagmo/archipelago.hpp"
#include "pagmo/problem.hpp"
#include "pagmo/algorithms/de.hpp"
#include "pagmo/algorithms/gaco.hpp"
#include "pagmo/algorithms/maco.hpp"
#include "pagmo/algorithms/moead.hpp"
#include "pagmo/algorithms/moead_gen.hpp"
#include "pagmo/algorithms/nsga2.hpp"
#include "pagmo/algorithms/nspso.hpp"
#include "pagmo/algorithms/xnes.hpp"
#include "pagmo/problems/cec2014.hpp"
#include "pagmo/problems/zdt.hpp"
#include "pagmo/utils/multi_objective.hpp"

void islandTest::run_algorithm_on_problem(const pagmo::problem& problem, const std::vector<pagmo::algorithm>& algorithm)
{
    // Proof of concept - Metasolver via "round-robin" algorithm scheduler
    auto algorithmPtr = algorithm.begin();
    auto getAlgorithm = [&]()
    {
        if (algorithmPtr == algorithm.end())
        {
            algorithmPtr = algorithm.begin();
        }

        const auto& alg = *algorithmPtr;
        std::advance(algorithmPtr, 1);
        std::cout << "Choosing algorithm: " << alg.get_name() << std::endl;
        return alg;
    };

    // TODO: Set-up topology in constructor
    pagmo::archipelago archi{};

    // Construct 8 distributed islands and add them to archipelago
    for (int i = 0; i < 8; ++i)
    {
        pagmo::distributed_island dist_island{};
        archi.push_back(pagmo::island{dist_island, getAlgorithm(), problem, POPULATION_SIZE});
    }

    // Run evolution 2 times on each island of this archi
    archi.evolve(1);
    // Wait for evolution to finish
    archi.wait_check();

    std::cout << "Main Archipelago: Evolution finished" << std::endl;
    if (problem.get_nobj() > 1)
    {
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
    else
    {
        std::cout << "Island champions:" << std::endl;
        const auto islCs = archi.get_champions_x();
        for (int i = 0; i < islCs.size(); ++i)
        {
            const auto& islC = islCs.at(i);
            std::cout << "[ ";
            for (const double val : islC)
            {
                std::cout << val << ", ";
            }
            std::cout << "]" << std::endl;
        }
    }
}

void islandTest::run_zdt(const std::vector<pagmo::algorithm>& algorithm)
{
    for (int i = 1; i <= 1; ++i) //6
    {
        //std::cout << "-------------------------------------------------------" << std::endl;
        //std::cout << "ZDT problem No." << i << std::endl;
        const pagmo::problem prob{pagmo::zdt(i)};
        run_algorithm_on_problem(prob, algorithm);
    }
}

void islandTest::run_cec2014(const std::vector<pagmo::algorithm>& algorithm)
{
    //std::cout << std::endl << "Running CEC2014 for algorithm: " << algorithm.get_name() << std::endl;

    // This runs each of the CEC2014 problems for this algorithm in archipelago mode
    for (int i = 1; i <= 30; ++i)
    {
        //std::cout << "-------------------------------------------------------" << std::endl;
        //std::cout << "CEC2014 problem No." << i << std::endl;
        const pagmo::problem prob{pagmo::cec2014(i, 30)};
        run_algorithm_on_problem(prob, algorithm);
    }
}

void islandTest::run_dll_problem(const std::vector<pagmo::algorithm>& algorithm)
{
    lib_loader<base_problem> loader{"./problems" + portable_dll_extension()};
    loader.open_lib();
    const std::shared_ptr<base_problem> baseProb = loader.get_instance();
    dll_problem_wrapper probWrapper{baseProb, baseProb->get_lib_file_name()};

    const pagmo::problem prob{probWrapper};
    run_algorithm_on_problem(prob, algorithm);
}


void islandTest::run_nsga2(const std::function<void(const std::vector<pagmo::algorithm>&)>& problemRunner)
{
    pagmo::algorithm nsga2{pagmo::nsga2(GEN_COUNT)};
    nsga2.set_verbosity(0);
    problemRunner({nsga2});
}


void islandTest::run_de(const std::function<void(const std::vector<pagmo::algorithm>&)>& problemRunner)
{
    pagmo::algorithm algo{pagmo::de(GEN_COUNT)};
    algo.set_verbosity(0);
    problemRunner({algo});
}

void islandTest::run_gaco(const std::function<void(const std::vector<pagmo::algorithm>&)>& problemRunner)
{
    pagmo::algorithm algo{pagmo::gaco(GEN_COUNT)};
    algo.set_verbosity(0);
    problemRunner({algo});
}


void islandTest::run_meta_multiobjective(const std::function<void(const std::vector<pagmo::algorithm>&)>& problemRunner)
{
    pagmo::algorithm nsga2{pagmo::nsga2(GEN_COUNT)};
    pagmo::algorithm moead{pagmo::moead(GEN_COUNT)};
    pagmo::algorithm moead_gen{pagmo::moead_gen(GEN_COUNT)};
    pagmo::algorithm maco{pagmo::maco(GEN_COUNT)};
    pagmo::algorithm nspso{pagmo::nspso(GEN_COUNT)};

    nsga2.set_verbosity(0);
    problemRunner({nsga2, moead, moead_gen, maco, nspso});
}
