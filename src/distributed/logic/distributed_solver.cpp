#include "distributed_solver.h"

#include "aixlog.hpp"
#include "distributed_island.h"
#include "population_tools.h"
#include "pagmo/utils/multi_objective.hpp"

void distributed_solver::_set_island_hints(pagmo::island& isl) const
{
    if (!_initialHints.empty())
    {
        auto pop = isl.get_population();
        const size_t popSize = pop.size();
        for (size_t i = 0; i < std::min(_initialHints.size(), popSize); i++)
        {
            pop.set_x(i, _initialHints.at(i));
        }
        isl.set_population(pop);
    }
}

std::vector<std::tuple<size_t, size_t, std::string, const pagmo::algorithm&>> distributed_solver::_generate_work_plan(
    const size_t islandCount, const size_t populationSize, const std::vector<pagmo::algorithm>& algorithms, const size_t
    minIslandPopSize)
{
    std::vector<std::tuple<size_t, size_t, std::string, const pagmo::algorithm&>> output{};

    // Helper lambda - "round-robin" algorithm chooser
    auto algorithmPtr = algorithms.begin();
    auto getAlgorithm = [&]() -> const pagmo::algorithm&
    {
        if (algorithmPtr == algorithms.end())
            algorithmPtr = algorithms.begin();

        const auto& alg = *algorithmPtr++;
        LOG(TRACE) << "Choosing algorithm: " << alg.get_name() << std::endl;
        return alg;
    };

    // Helper lambda - safely compute average preventing division by zero
    auto safeAvg = [](const uint64_t processed, const uint64_t time) -> double
    {
        return (time > 0) ? (static_cast<double>(processed) / static_cast<double>(time)) : 0;
    };

    // Prepare worker-related vars
    auto& repo = _controller.get_worker_info_repository();
    std::unordered_set<std::string> connectedWorkers = repo.get_connected_workers();
    const size_t workerCount = connectedWorkers.size();

    // OPTION 1: There are more islands than connected workers (they've not connected yet), so divide work equally
    if (islandCount > workerCount)
    {
        const size_t islandPop = std::max(minIslandPopSize, populationSize / islandCount);
        for (size_t i = 0; i < islandCount; ++i)
        {
            // Empty worker id, it doesn't matter because all workers get the same amount of work
            output.emplace_back(islandPop, 1, "", getAlgorithm());
        }
    }

    // OPTION 2: All expected workers have connected, we can divide work by their performance, there can be extra workers too
    else
    {
        // A vector of [workerId, algorithm, performanceAvgMetric], this is used to calculate island populations
        std::vector<std::tuple<std::string, const pagmo::algorithm&, double>> workersPreprocessed{};
        workersPreprocessed.reserve(islandCount);

        // 1) Assign workers to algorithms, picking the best-fit worker per island
        for (size_t i = 0; i < islandCount; ++i)
        {
            const auto& currentAlgorithm = getAlgorithm();
            const auto& algoName = currentAlgorithm.get_name();

            double bestTotalAvg = 0;
            std::string bestTotalWorkerId;
            double bestAlgoAvg = 0;
            std::string bestAlgoWorkerId;

            // Find the best worker in total together with the best worker for this algorithm
            for (const auto& workerId : connectedWorkers)
            {
                const auto& wInfo = repo.get_worker_info(workerId);

                const double wTotalAvg = safeAvg(
                    wInfo->totalStats.processedPopulation,
                    wInfo->totalStats.workTime
                );

                double wAlgoAvg = 0;
                if (wInfo->statsByAlgorithm.contains(algoName))
                {
                    const auto& algoStats = wInfo->statsByAlgorithm.at(algoName);
                    wAlgoAvg = safeAvg(
                        algoStats.processedPopulation,
                        algoStats.workTime
                    );
                }

                if (wTotalAvg >= bestTotalAvg)
                {
                    bestTotalAvg = wTotalAvg;
                    bestTotalWorkerId = workerId;
                }

                if (wAlgoAvg >= bestAlgoAvg)
                {
                    bestAlgoAvg = wAlgoAvg;
                    bestAlgoWorkerId = workerId;
                }
            }

            // We prefer to choose by algorithm stats, we choose by total only if it's 2x better
            const bool preferTotal = (bestTotalAvg > 2 * bestAlgoAvg);
            const auto& chosenWorkerId = preferTotal ? bestTotalWorkerId : bestAlgoWorkerId;
            const double chosenAvg = preferTotal ? bestTotalAvg : bestAlgoAvg;

            workersPreprocessed.emplace_back(chosenWorkerId, currentAlgorithm, chosenAvg);
            // Remove the chosen worker from the set so it cannot be assigned to another island
            connectedWorkers.erase(chosenWorkerId);
        }

        // 2) Calculate the total performance of the worker cluster
        double totalPerformance = 0;
        for (const auto& [workerId, algorithm, perfMetric] : workersPreprocessed)
        {
            totalPerformance += perfMetric;
        }

        // 3) Calculate the percentage of performance committed by each worker, pop size is then derived from this
        for (const auto& [workerId, algorithm, perfMetric] : workersPreprocessed)
        {
            double workerPerfPercentage;
            if (totalPerformance > 0)
            {
                workerPerfPercentage = static_cast<double>(perfMetric) / static_cast<double>(totalPerformance);
            } else
            {
                // If all metrics are zero fall back to an equal distribution to avoid division by zero
                workerPerfPercentage = 1.0 / static_cast<double>(workersPreprocessed.size());
            }

            // The population size of each worker is proportional to the percentage of performance it has
            const auto workerPopSize = static_cast<size_t>(static_cast<double>(populationSize) * workerPerfPercentage);

            output.emplace_back(std::max(minIslandPopSize, workerPopSize), 1, workerId, algorithm);
        }
    }

    return output;
}

distributed_solver::distributed_solver(const std::string& controllerAddress, const size_t expectedWorkerCount) :
    _controller(controllerAddress), _expectedWorkerCount(expectedWorkerCount)
{
    _controller.run_server();
}

void distributed_solver::evolve(const pagmo::problem& problem, const std::vector<pagmo::algorithm>& algorithms,
                                const size_t populationSize, const size_t cycleCount, const size_t minIslandPopSize)
{
    // This will prevent interrupting the archipelago if it's already evolving
    if (_archipelago.status() == pagmo::evolve_status::busy ||
        _archipelago.status() == pagmo::evolve_status::busy_error)
    {
        throw std::runtime_error("Cannot start a new evolution while the previous one is still running.");
    }

    // TODO: Set-up topology in constructor
    _archipelago = pagmo::archipelago{};

    const size_t currentWorkerCount = _controller.get_worker_info_repository().get_worker_count();
    // If there are more workers connected to controller than what was expected, we increase the island count
    const size_t optimalIslandCount = std::max(currentWorkerCount, _expectedWorkerCount);

    // Work plan ensures correct worker load balancing
    const auto& workPlan = _generate_work_plan(optimalIslandCount, populationSize, algorithms, minIslandPopSize);

    for (int i = 0; i < optimalIslandCount; ++i)
    {
        const auto [islandPopSize,islandCycleCount,preferredWorker, algorithm] = workPlan.at(i);

        pagmo::distributed_island dist_island{preferredWorker, islandCycleCount};
        auto isl = pagmo::island{dist_island, algorithm, problem, islandPopSize};
        // We set the initial population to this island (hints) if there are any
        _set_island_hints(isl);
        _archipelago.push_back(isl);
    }

    // Start evolving
    _archipelago.evolve(cycleCount);
    // Clear the hints so they're not re-used next time
    _initialHints.clear();
}

void distributed_solver::set_initial_hints(const std::vector<pagmo::vector_double>& hints)
{
    _initialHints = hints;
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
    for (const double n : bestIndividual)
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
    return (!best.empty()) ? best.at(0) : pagmo::vector_double{};
}

pagmo::evolve_status distributed_solver::get_status() const
{
    return _archipelago.status();
}


std::vector<pagmo::vector_double> distributed_solver::get_best_N_individuals(const size_t N)
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

void distributed_solver::wait_until_workers_connect(const size_t workerCount)
{
    _controller.get_worker_info_repository().wait_until_worker_count(workerCount);
}

void distributed_solver::wait_until_workers_connect()
{
    wait_until_workers_connect(_expectedWorkerCount);
}
