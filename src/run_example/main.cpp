#include <utility>

#include "aixlog.hpp"
#include "distributed_controller.h"
#include "distributed_solver.h"
#include "distributed_worker.h"
#include "population_tools.h"
#include "udp_dll_wrapper.h"
#include "udp_registry.h"
#include "pagmo/algorithms/gaco.hpp"
#include "pagmo/problems/zdt.hpp"


class solver_benchmark
{
    pagmo::problem _problem;
    std::vector<pagmo::vector_double> _dataPoints;
    std::vector<std::string> _pointIDs;

public:
    explicit solver_benchmark(pagmo::problem problem)
        : _problem(std::move(problem))
    {
    }

    void add_result(pagmo::vector_double result, std::string pointID)
    {
        _dataPoints.emplace_back(std::move(result));
        _pointIDs.emplace_back(std::move(pointID));
    }

std::string get_benchmark_statistics()
{
    std::string output;
    const size_t nObj = _problem.get_nobj();
    const bool isMultiObjective = nObj > 1;

    // 1) Calculate fitnesses of all data points
    std::vector<pagmo::vector_double> fitnesses;
    fitnesses.reserve(_dataPoints.size());
    for (const auto& point : _dataPoints)
    {
        fitnesses.emplace_back(_problem.fitness(point));
    }

    // 2) Get indexes of data points sorted from best to worst (works for both MO and SO)
    const std::vector<pagmo::pop_size_t> bestIndividualIndexes = get_best_N_individuals_indexes(
        _problem, fitnesses, _dataPoints.size()
    );

    // 3) Sort the data points, their IDs and fitnesses
    std::vector<pagmo::vector_double> sortedPoints;
    std::vector<pagmo::vector_double> sortedFitnesses;
    std::vector<std::string> sortedPointIDs;
    for (const size_t idx : bestIndividualIndexes)
    {
        sortedPoints.emplace_back(_dataPoints[idx]);
        sortedFitnesses.emplace_back(fitnesses[idx]);
        sortedPointIDs.emplace_back(_pointIDs[idx]);
    }

    // 4) Multi-objective pre-calculations (ideal point and ranks)
    pagmo::vector_double ideal(nObj, std::numeric_limits<double>::max());
    pagmo::vector_double nadir(nObj, -std::numeric_limits<double>::max());
    std::vector<pagmo::pop_size_t> ranks, dom_counts;
    std::vector<double> crowdDists;

    if (isMultiObjective) {
        // Find min/max for normalization
        for (const auto& f : sortedFitnesses) {
            for (size_t j = 0; j < nObj; ++j) {
                if (f[j] < ideal[j]) ideal[j] = f[j];
                if (f[j] > nadir[j]) nadir[j] = f[j];
            }
        }
        // Calculate MO metrics
        if (sortedFitnesses.size() >= 2) {
            auto fnds = pagmo::fast_non_dominated_sorting(sortedFitnesses);
            dom_counts = std::get<2>(fnds);
            ranks = std::get<3>(fnds);
            crowdDists = pagmo::crowding_distance(sortedFitnesses);
        }
    }

    // 5) Build header
    output += "ID\t\t\tValue\t\t\t\tFitness\t\t\t\tIncrease over baseline";
    if (isMultiObjective) {
        output += "\t\tRank\tDomBy\tCrowdDist\tDistToIdeal";
    }
    output += "\n";

    // 6) Build rows
    const auto bestIndividualFitness = sortedFitnesses[0];
    for (int i = 0; i < sortedPoints.size(); ++i)
    {
        const auto& point = sortedPoints[i];
        const auto& fitness = sortedFitnesses[i];
        const auto& pointID = sortedPointIDs[i];

        output += pointID + "\t\t\t" + double_vector_to_str(point) + "\t\t\t" + double_vector_to_str(fitness) + "\t\t\t";

        // Calculate percentage difference within fitness vectors
        output += "[";
        for (int j = 0; j < bestIndividualFitness.size(); ++j)
        {
            double percentageDecline = 0.0;
            if (bestIndividualFitness[j] != 0.0)
            {
                double difference = fitness[j] - bestIndividualFitness[j];
                percentageDecline = difference / bestIndividualFitness[j];
            }
            std::string percentage = std::to_string(percentageDecline * 100);
            percentage = percentage.substr(0, percentage.find(".") + 3);
            output += percentage + "%" + ", ";
        }
        output.pop_back();
        output.back() = ']';

        // Append MO specific columns
        if (isMultiObjective && !ranks.empty())
        {
            // Calculate Normalized Distance to Ideal for this specific point
            double distToIdeal = 0.0;
            for (size_t j = 0; j < nObj; ++j) {
                double range = nadir[j] - ideal[j];
                double norm = (range > 1e-9) ? (fitness[j] - ideal[j]) / range : 0.0;
                distToIdeal += norm * norm;
            }
            distToIdeal = std::sqrt(distToIdeal);

            output += "\t\t" + std::to_string(ranks[i]) +
                      "\t" + std::to_string(dom_counts[i]) +
                      "\t" + (std::isinf(crowdDists[i]) ? "Inf" : std::to_string(crowdDists[i]).substr(0, 5)) +
                      "\t" + std::to_string(distToIdeal).substr(0, 5);
        }

        output += "\n";
    }

    return output;
}
};


int main(int argc, char* argv[])
{
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);

    std::string address = "tcp://localhost:5000";

    if (argc >= 2 && argv[1] == std::string("-run-controller"))
    {

        /*
        udp_registry::get().set_local_cache_dir("controller_cache");

        udp_dll_wrapper probWrapper{"schwefel_udp"};
        const pagmo::problem prob{probWrapper};

        solver_benchmark s{prob};
        s.add_result({419.9}, "Second");
        s.add_result({419.8}, "Third");
        s.add_result({420}, "First");
        s.add_result({419.7}, "Fourth");
        */
        const pagmo::problem prob{pagmo::zdt{1}};


        solver_benchmark s{prob};

        {
            const pagmo::population pop{prob, 100};
            const pagmo::nsga2 algo{50};
            const auto out = algo.evolve(pop);
            const auto best = select_best_individual(prob, out.get_x(), out.get_f());
            s.add_result(best, "First");
        }

        {
            const pagmo::population pop{prob, 60};
            const pagmo::nsga2 algo{50};
            const auto out = algo.evolve(pop);
            const auto best = select_best_individual(prob, out.get_x(), out.get_f());
            s.add_result(best, "Second");
        }

        {
            const pagmo::population pop{prob, 40};
            const pagmo::nsga2 algo{50};
            const auto out = algo.evolve(pop);
            const auto best = select_best_individual(prob, out.get_x(), out.get_f());
            s.add_result(best, "Third");
        }


        std::cout<<"Here"<<std::endl;
        std::cout << s.get_benchmark_statistics() << std::endl;

        // solver_benchmark s{prob};
        // s.add_result({419.9}, "Second");
        // s.add_result({419.8}, "Third");
        // s.add_result({420}, "First");
        // s.add_result({419.7}, "Fourth");
        //
        // std::cout << s.get_benchmark_statistics() << std::endl;


        // pagmo::algorithm algo{pagmo::gaco(1500)};
        // algo.set_verbosity(0);
        //
        // distributed_solver ds{address};
        // ds.evolve(prob, {algo}, 100);
        // ds.wait_until_completion();
    }
    else
    {
        distributed_worker worker{address};
        // We register this worker with the UDP registry, so DLLs can be requested from controller
        udp_registry::get().set_local_cache_dir("worker_cache");
        udp_registry::get().register_udp_provider(
            [&worker](const std::string& libName)
            {
                return worker.get_dll_from_controller(libName);
            }
        );

        for (;;)
        {
            worker.client_loop();
        }
    }

    return 0;
}
