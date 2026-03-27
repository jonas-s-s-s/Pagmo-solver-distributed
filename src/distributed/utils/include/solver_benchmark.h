#pragma once
#include "population_tools.h"
#include "pagmo/problem.hpp"
#include "pagmo/utils/hypervolume.hpp"

class solver_benchmark
{
    pagmo::problem _problem;
    std::vector<pagmo::vector_double> _dataPoints;
    std::vector<std::string> _pointIDs;
    std::vector<double> _elapsedTimes;

    std::chrono::high_resolution_clock::time_point _timerStart;
public:
    explicit solver_benchmark(pagmo::problem problem)
        : _problem(std::move(problem))
    {
    }

    void start_timer() {
        _timerStart = std::chrono::high_resolution_clock::now();
    }

    double stop_timer() const {
        const auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - _timerStart).count();
    }

    void add_result(pagmo::vector_double result, std::string pointID, double elapsedTime)
    {
        _dataPoints.emplace_back(std::move(result));
        _pointIDs.emplace_back(std::move(pointID));
        _elapsedTimes.emplace_back(elapsedTime);
    }

    std::string get_benchmark_stats_csv()
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

        // 3) Sort the data points, their IDs, fitnesses and elapsed times
        std::vector<pagmo::vector_double> sortedPoints;
        std::vector<pagmo::vector_double> sortedFitnesses;
        std::vector<std::string> sortedPointIDs;
        std::vector<double> sortedElapsedTimes;
        for (const size_t idx : bestIndividualIndexes)
        {
            sortedPoints.emplace_back(_dataPoints[idx]);
            sortedFitnesses.emplace_back(fitnesses[idx]);
            sortedPointIDs.emplace_back(_pointIDs[idx]);
            sortedElapsedTimes.emplace_back(_elapsedTimes[idx]);
        }

        // 4) Multi-objective calculations (ranks + hypervolume)
        std::vector<pagmo::pop_size_t> ranks;
        std::vector<pagmo::pop_size_t> domCounts;
        std::vector<double> crowdDists;
        std::vector<double> hvContributions;

        if (isMultiObjective)
        {
            // Calculate MO metrics
            if (sortedFitnesses.size() >= 2)
            {
                auto fnds = pagmo::fast_non_dominated_sorting(sortedFitnesses);
                domCounts = std::get<2>(fnds);
                ranks = std::get<3>(fnds);
                crowdDists = pagmo::crowding_distance(sortedFitnesses);

                // Hypervolume contributions
                pagmo::hypervolume hv(sortedFitnesses);
                auto ref = hv.refpoint(1.0);
                hvContributions = hv.contributions(ref);
            }
        }

        // 5) Build header
        output += "ID,Value,Fitness,Fitness increase over baseline,ElapsedTime";
        if (isMultiObjective)
        {
            output += ",Rank,DomBy,CrowdDist,HVContribution";
        }
        output += "\n";

        // 6) Build rows
        const auto bestIndividualFitness = sortedFitnesses[0];
        for (int i = 0; i < sortedPoints.size(); ++i)
        {
            const auto& point = sortedPoints[i];
            const auto& fitness = sortedFitnesses[i];
            const auto& pointID = sortedPointIDs[i];

            output += pointID + "," +
                double_vector_to_csv(point) + "," +
                double_vector_to_csv(fitness) + ",";

            // Calculate percentage difference within fitness vectors
            output += "\"";
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
                output += percentage + "%";
                if (j + 1 < bestIndividualFitness.size())
                {
                    output += ";";
                }
            }
            output += "\"";

            // Append elapsed time
            output += "," + std::to_string(sortedElapsedTimes[i]);

            // Append MO specific columns
            if (isMultiObjective && !ranks.empty())
            {
                output += "," + std::to_string(ranks[i]) +
                    "," + std::to_string(domCounts[i]) +
                    "," + (std::isinf(crowdDists[i]) ? "Inf" : std::to_string(crowdDists[i]).substr(0, 5)) +
                    "," + std::to_string(hvContributions[i]).substr(0, 8);
            }

            output += "\n";
        }

        return output;
    }
};