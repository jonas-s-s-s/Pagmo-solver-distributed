#pragma once
#include "distributed_solver.h"
#include "distributed_worker.h"
#include "benchmark_stats.h"

#include "pagmo/algorithms/bee_colony.hpp"
#include "pagmo/algorithms/de.hpp"
#include "pagmo/algorithms/de1220.hpp"
#include "pagmo/algorithms/gaco.hpp"
#include "pagmo/algorithms/gwo.hpp"
#include "pagmo/algorithms/pso.hpp"
#include "pagmo/algorithms/sade.hpp"
#include "pagmo/algorithms/simulated_annealing.hpp"

#include <iostream>

#include "benchmark_stats.h"
#include "pagmo/problems/ackley.hpp"
#include "pagmo/problems/griewank.hpp"
#include "pagmo/problems/rastrigin.hpp"
#include "pagmo/problems/rosenbrock.hpp"
#include "pagmo/problems/schwefel.hpp"

size_t get_so_algorithm_count();

std::vector<pagmo::algorithm> construct_so_algorithms(size_t genCount);

void benchmark_compare_so(distributed_solver& ds,
                          const pagmo::problem& prob,
                          benchmark_stats& bench,
                          size_t popSize,
                          size_t genCount);

void run_so_benchmark(distributed_solver& ds,
                      benchmark_stats& bench,
                      bool outputHtml = true,
                      bool outputCsv = true,
                      size_t popSize = 3200,
                      size_t genCount = 1000);
