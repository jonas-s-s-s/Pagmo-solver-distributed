#include "distributed_solver.h"
#include "distributed_worker.h"
#include "pagmo/algorithms/de.hpp"
#include <string>
#include "benchmark_stats.h"
#include "so_benchmark.h"
#include "pagmo/algorithms/cmaes.hpp"

int main()
{
    // Change to pagmo::rosenbrock here for the second phase
    pagmo::problem problem{pagmo::rastrigin{60}};
    benchmark_stats bench{problem};
    int gens = 750;
    pagmo::algorithm algo{pagmo::de(gens)};
    // 1) Running DE on 3 worker nodes
    {
        distributed_solver solver{
            "tcp://0.0.0.0:5001",
            3,
        };
        solver.enable_logging();
        solver.wait_until_workers_connect();
        bench.start_timer();
        // Total population size of 10 000, split between 3 workers, 5 migration cycles
        solver.evolve(problem, {algo}, 10000, 5);
        solver.wait_until_completion();
        bench.add_data_point(
            solver.get_best_individual(),
            "NORMAL_" + algo.get_name(),
            bench.stop_timer()
        );
    }
    // 2) Running {DE, CMA-ES, PSO} on 3 worker nodes
    pagmo::algorithm de{pagmo::de(gens)};
    pagmo::algorithm cmaes{pagmo::cmaes(gens)};
    pagmo::algorithm pso{pagmo::pso(gens)};
    auto algs = std::vector{de, cmaes, pso};
    {
        distributed_solver solver{
            "tcp://0.0.0.0:5001",
            3
        };
        solver.enable_logging();
        solver.wait_until_workers_connect();
        bench.start_timer();
        // Total population size of 10 000, split between 3 workers, 5 migration cycles
        solver.evolve(problem, {algs}, 10000, 5);
        solver.wait_until_completion();
        bench.add_data_point(
            solver.get_best_individual(),
            "METASOLVER",
            bench.stop_timer()
        );
    }
    // 3) Saving collected statistics
    bench.end_current_measurement();
    bench.save_all_results_as_csv();
    return 0;
}
