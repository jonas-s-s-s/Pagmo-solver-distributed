#pragma once
#include "distributed_solver.h"
#include "distributed_worker.h"
#include "benchmark_stats.h"

#include "pagmo/algorithms/ihs.hpp"
#include "pagmo/algorithms/maco.hpp"
#include "pagmo/algorithms/moead.hpp"
#include "pagmo/algorithms/moead_gen.hpp"
#include "pagmo/algorithms/nsga2.hpp"
#include "pagmo/algorithms/nspso.hpp"

#include "pagmo/problems/dtlz.hpp"
#include "pagmo/problems/wfg.hpp"
#include "pagmo/problems/zdt.hpp"

#include <iostream>

size_t get_mo_algorithm_count();

std::vector<pagmo::algorithm> construct_mo_algorithms(size_t genCount);

void benchmark_compare_mo(distributed_solver& ds,
                          const pagmo::problem& prob,
                          benchmark_stats& bench,
                          size_t popSize,
                          size_t genCount);

void run_mo_benchmark(distributed_solver& ds,
                      benchmark_stats& bench,
                      bool outputHtml = true,
                      bool outputCsv  = true,
                      size_t popSize  = 500,
                      size_t genCount = 500);