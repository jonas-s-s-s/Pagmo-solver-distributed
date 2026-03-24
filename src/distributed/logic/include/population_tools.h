#pragma once
#include <tuple>
#include <vector>
#include <optional>

#include "pagmo/archipelago.hpp"
#include "pagmo/types.hpp"
#include "pagmo/utils/multi_objective.hpp"
#include "pagmo/utils/constrained.hpp"

/**
 * Helper function to merge the population of all islands (and their fitness) into two vectors
 * @param archi Input archipelago containing islands
 * @return Tuple of [allPopulations, allFitnesses]
 */
inline std::tuple<std::vector<pagmo::vector_double>, std::vector<pagmo::vector_double>> merge_populations(
    const pagmo::archipelago& archi)
{
    std::vector<pagmo::vector_double> allPopulations{};
    std::vector<pagmo::vector_double> allFitnesses{};
    for (const auto& isl : archi)
    {
        auto islPop = isl.get_population().get_x();
        auto islFit = isl.get_population().get_f();
        allPopulations.insert(allPopulations.end(), islPop.begin(), islPop.end());
        allFitnesses.insert(allFitnesses.end(), islFit.begin(), islFit.end());
    }

    return {allPopulations, allFitnesses};
}

/**
 * Selects best N individuals of a single objective problem
 * @returns Indexes of the best N individuals
 */
inline std::vector<pagmo::pop_size_t> select_best_N_so(const pagmo::problem& prob,
                                                       const std::vector<pagmo::vector_double>& fitness,
                                                       const std::size_t N)
{
    if (prob.get_nc() == 0)
    {
        // Non-constrained version
        std::vector<pagmo::pop_size_t> idx(fitness.size());
        std::iota(idx.begin(), idx.end(), static_cast<pagmo::pop_size_t>(0));

        std::sort(idx.begin(), idx.end(),
                  [&fitness](auto a, auto b)
                  {
                      return fitness[a][0] < fitness[b][0];
                  });

        idx.resize(N);
        return idx;
    }

    // Constrained version
    auto idx = pagmo::sort_population_con(fitness, prob.get_nec(), prob.get_c_tol());
    idx.resize(N);
    return idx;
}

/**
 * Creates a single new pagmo::population, based on individuals provided in allPopulations and selectedIndexes
 * @param prob Needs to be provided, pagmo::problem is a part of pagmo::population
 * @param allPopulations Vector of individuals which is bigger than the new population
 * @param selectedIndexes Indexes of individuals we want to include in the new population
 * @param seed Optional seed for the pagmo::population
 * @return A new population containing only those individuals specified by selectedIndexes
 */
inline pagmo::population build_new_population(
    const pagmo::problem& prob,
    const std::vector<pagmo::vector_double>& allPopulations,
    const std::vector<pagmo::pop_size_t>& selectedIndexes,
    const std::optional<unsigned> seed = std::nullopt)
{
    //  Construct the new population object using provided arguments
    pagmo::population newPop{
        prob,
        selectedIndexes.size(),
        seed.value_or(pagmo::random_device::next())
    };

    // Fill the newPop object with new individuals ("best" POPULATION_SIZE individuals)
    for (std::size_t i = 0; i < selectedIndexes.size(); ++i)
    {
        const auto idx = selectedIndexes[i];
        newPop.set_x(i, allPopulations[idx]);
    }

    return newPop;
}

/**
 * - Selects best N individuals from allPopulations, then returns a new pagmo::population containing only them
 * - Workflow of this function is:
 *  - 1) Determine if the problem is SO or MO
 *  - 2) Find out indexes of best N individuals (SO or MO version)
 *  - 3) Construct a new population object (build_new_population)
 *  - 4) Fill the new population object according to indexes from 2) (build_new_population)
 *  - 5) Return the final pagmo::population
 * @param prob Needed to determine if the problem is SO or MO, and to construct pagmo::population
 * @param allPopulations Individuals of all the populations we want to choose from
 * @param allFitness Fitnesses of allPopulations
 * @param N Size of the output population
 * @param seed Optional seed for the pagmo::population
 * @return A new population of size N, containing only the best N individuals from allPopulations
 */
inline pagmo::population select_best_N_into_new_population(const pagmo::problem& prob,
                                                           const std::vector<pagmo::vector_double>& allPopulations,
                                                           const std::vector<pagmo::vector_double>& allFitness,
                                                           std::size_t N,
                                                           const std::optional<unsigned> seed = std::nullopt)
{
    const bool isMultiObjective = prob.get_nobj() > 1;
    std::vector<pagmo::pop_size_t> bestIndividualIndexes;
    // Individuals in a population need to be sorted differently depending on if it's a multi-objective or single-objective problem
    if (isMultiObjective)
    {
        bestIndividualIndexes = pagmo::select_best_N_mo(allFitness, N);
    }
    else
    {
        bestIndividualIndexes = select_best_N_so(prob, allFitness, N);
    }

    return build_new_population(prob, allPopulations, bestIndividualIndexes, seed);
}

/**
 * Get indexes of best N individuals from the provided allFitnesses vector
 */
inline std::vector<pagmo::pop_size_t> get_best_N_individuals_indexes(const pagmo::problem& prob,
                                                                     const std::vector<pagmo::vector_double>& allFitness,
                                                                     std::size_t N)
{
    const bool isMultiObjective = prob.get_nobj() > 1;
    std::vector<pagmo::pop_size_t> bestIndividualIndexes;
    // Individuals in a population need to be sorted differently depending on if it's a multi-objective or single-objective problem
    if (isMultiObjective)
    {
        bestIndividualIndexes = pagmo::select_best_N_mo(allFitness, N);
    }
    else
    {
        bestIndividualIndexes = select_best_N_so(prob, allFitness, N);
    }

    return bestIndividualIndexes;
}

/**
 * Selects best N individuals from allPopulations
 */
inline std::vector<pagmo::vector_double> select_best_N_individuals(const pagmo::problem& prob,
                                                                   const std::vector<pagmo::vector_double>&
                                                                   allPopulations,
                                                                   const std::vector<pagmo::vector_double>& allFitness,
                                                                   std::size_t N)
{
    std::vector<pagmo::vector_double> output{};
    const std::vector<pagmo::pop_size_t> bestIndividualIndexes = get_best_N_individuals_indexes(prob, allFitness, N);

    // Fill the output object
    for (const size_t idx : bestIndividualIndexes)
    {
        output.emplace_back(allPopulations[idx]);
    }

    return output;
}

/**
 * Selects THE best individual from allPopulations
 */
inline pagmo::vector_double select_best_individual(const pagmo::problem& prob,
                                                   const std::vector<pagmo::vector_double>& allPopulations,
                                                   const std::vector<pagmo::vector_double>& allFitness)
{
    return select_best_N_individuals(prob, allPopulations, allFitness, 1).at(0);
}

inline std::string double_vector_to_str(const pagmo::vector_double& vec)
{
    std::string out = "[";
    for (const double n : vec)
    {
        out += std::to_string(n) + ", ";
    }
    out.pop_back();
    out.back() = ']';

    return out;
}

inline std::string double_vector_to_csv(const pagmo::vector_double& vec)
{
    std::string out;
    out += "\"";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        out += std::to_string(vec[i]);
        if (i + 1 < vec.size())
        {
            out += ";";
        }
    }
    out += "\"";
    return out;
}