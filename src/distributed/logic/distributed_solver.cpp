#include "distributed_solver.h"

#include "aixlog.hpp"
#include "distributed_island.h"
#include "population_tools.h"
#include "pagmo/utils/multi_objective.hpp"

distributed_solver::distributed_solver(const std::string& controllerAddress, const size_t expectedWorkerCount) :
    _controller(controllerAddress), _expectedWorkerCount(expectedWorkerCount)
{
    _controller.run_server();
}

void distributed_solver::evolve(const pagmo::problem& problem, const std::vector<pagmo::algorithm>& algorithms,
                                const size_t populationSize, size_t cycleCount)
{
    // This will prevent interrupting the archipelago if it's already evolving
    if (_archipelago.status() == pagmo::evolve_status::busy ||
        _archipelago.status() == pagmo::evolve_status::busy_error)
    {
        throw std::runtime_error("Cannot start a new evolution while the previous one is still running.");
    }

    // Metasolver via "round-robin" algorithm chooser, each island gets a different algorithm
    auto algorithmPtr = algorithms.begin();
    auto getAlgorithm = [&]()
    {
        if (algorithmPtr == algorithms.end())
        {
            algorithmPtr = algorithms.begin();
        }

        const auto& alg = *algorithmPtr;
        std::advance(algorithmPtr, 1);
        LOG(TRACE) << "Choosing algorithm: " << alg.get_name() << std::endl;
        return alg;
    };

    // TODO: Set-up topology in constructor
    _archipelago = pagmo::archipelago{};

    const size_t currentWorkerCount = _controller.get_worker_info_repository().get_worker_count();
    // If there are more workers connected to controller than what was expected, we increase the island count
    const size_t optimalIslandCount = (_expectedWorkerCount > currentWorkerCount)
                                          ? _expectedWorkerCount
                                          : currentWorkerCount;

    for (int i = 0; i < optimalIslandCount; ++i)
    {
        pagmo::distributed_island dist_island{};
        _archipelago.push_back(pagmo::island{dist_island, getAlgorithm(), problem, populationSize});
    }

    _archipelago.evolve(cycleCount);
}

pagmo::vector_double distributed_solver::wait_until_completion()
{
    if (_archipelago.status() == pagmo::evolve_status::idle ||
        _archipelago.status() == pagmo::evolve_status::idle_error)
    {
        throw std::runtime_error("Cannot wait until completion, no evolution is running at the moment.");
    }

    _archipelago.wait_check();

    LOG(TRACE) << "Main Archipelago: Evolution finished" << std::endl;
    pagmo::vector_double bestIndividual = get_best_individual();

    std::string bestStr = "[";
    for (double n : bestIndividual)
    {
        bestStr += std::to_string(n);
        bestStr += ", ";
    }
    bestStr += "]";
    LOG(TRACE) << "Best individual: " << bestStr << std::endl;

    return bestIndividual;
}

pagmo::vector_double distributed_solver::get_best_individual()
{
    const auto best = get_best_N_individuals(1);
    return (best.size() > 0) ? best.at(0) : pagmo::vector_double{};
}


std::vector<pagmo::vector_double> distributed_solver::get_best_N_individuals(size_t N)
{
    if (_archipelago.size() == 0)
    {
        return {};
    }

    auto [allPopulations, allFitnesses] = merge_populations(_archipelago);
    auto bestIndividuals = select_best_N_individuals(
        _archipelago.begin()->get_population().get_problem(),
        allPopulations,
        allFitnesses,
        N
    );

    return bestIndividuals;
}

void distributed_solver::wait_until_workers_connect(size_t workerCount)
{
    _controller.get_worker_info_repository().wait_until_worker_count(workerCount);
}

void distributed_solver::wait_until_workers_connect()
{
    wait_until_workers_connect(_expectedWorkerCount);
}
