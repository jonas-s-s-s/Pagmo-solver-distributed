#include "aixlog.hpp"
#include "distributed_controller.h"
#include "distributed_solver.h"
#include "distributed_worker.h"
#include "../distributed/benchmark/include/solver_benchmark.h"
#include "udp_dll_wrapper.h"
#include "udp_registry.h"

#include "pagmo/algorithms/bee_colony.hpp"
#include "pagmo/algorithms/de.hpp"
#include "pagmo/algorithms/de1220.hpp"
#include "pagmo/algorithms/gaco.hpp"
#include "pagmo/algorithms/gwo.hpp"
#include "pagmo/algorithms/pso.hpp"
#include "pagmo/algorithms/sade.hpp"
#include "pagmo/algorithms/simulated_annealing.hpp"

#include <thread>
#include <iostream>

#include "csv_conversion.h"
#include "pagmo/problems/ackley.hpp"
#include "pagmo/problems/griewank.hpp"
#include "pagmo/problems/rastrigin.hpp"
#include "pagmo/problems/rosenbrock.hpp"
#include "pagmo/problems/schwefel.hpp"

void benchmark_dist_solver_compare(const std::string& address,
                                   const pagmo::problem& prob,
                                   const size_t popSize = 1000,
                                   const size_t genCount = 500)
{
    // Prepare SO algorithms
    pagmo::algorithm a1{pagmo::gaco(genCount)};
    pagmo::algorithm a2{pagmo::de(genCount)};
    pagmo::algorithm a3{pagmo::sade(genCount)};
    pagmo::algorithm a4{pagmo::de1220(genCount)};
    pagmo::algorithm a5{pagmo::gwo(genCount)};
    pagmo::algorithm a6{pagmo::pso(genCount)};
    pagmo::algorithm a7{pagmo::bee_colony(genCount)};
    pagmo::algorithm a8{pagmo::simulated_annealing(genCount)};

    a1.set_verbosity(0);
    a2.set_verbosity(0);
    a3.set_verbosity(0);
    a4.set_verbosity(0);
    a5.set_verbosity(0);
    a6.set_verbosity(0);
    a7.set_verbosity(0);
    a8.set_verbosity(0);

    std::vector<pagmo::algorithm> soAlgs = {a1, a2, a3, a4, a5, a6, a7, a8};
    const auto workerCount = soAlgs.size();

    distributed_solver ds{address, workerCount, load_balancing_strategy::ALL_ISLANDS_EQUAL};
    std::cout << "SO benchmark waiting for " << workerCount << " workers to connect..." << std::endl;
    ds.wait_until_workers_connect(workerCount);

    solver_benchmark bench{prob};

    // Run a benchmark for each SO algorithm VS metasolver using all algorithms
    for (int i = 0; i < soAlgs.size(); ++i)
    {
        std::cout << i + 1 << "/" << soAlgs.size() << " (" << soAlgs[i].get_name() + ")" << std::endl;

        // 1) Using only ONE algorithm
        {
            const auto& currentAlg = soAlgs[i];
            pagmo::algorithm algo{currentAlg};
            algo.set_verbosity(0);

            bench.start_timer();

            ds.evolve(prob, {algo}, popSize);
            ds.wait_until_completion();

            bench.add_data_point(
                ds.get_best_individual(),
                "SINGLE_" + currentAlg.get_name(),
                bench.stop_timer()
            );
        }

        // 2) Meta solving
        {
            bench.start_timer();

            ds.evolve(prob, soAlgs, popSize);
            ds.wait_until_completion();

            bench.add_data_point(
                ds.get_best_individual(),
                "META_ALL",
                bench.stop_timer()
            );
        }
    }

    bench.end_current_measurement();
    bench.save_all_results_as_html();
}

void run_benchmark_so(const std::string& address)
{
    std::cout << std::endl << "Running single objective benchmark..." << std::endl;

    std::cout << std::endl << "Running pagmo::schwefel..." << std::endl;
    {
        const pagmo::problem prob{pagmo::schwefel(2)};
        benchmark_dist_solver_compare(address, prob);
    }

    std::cout << std::endl << "Running pagmo::rosenbrock..." << std::endl;
    {
        pagmo::problem prob{pagmo::rosenbrock(30)};
        benchmark_dist_solver_compare(address, prob);
    }

    std::cout << std::endl << "Running pagmo::rastrigin..." << std::endl;
    {
        pagmo::problem prob{pagmo::rastrigin(30)};
        benchmark_dist_solver_compare(address, prob);
    }

    std::cout << std::endl << "Running pagmo::ackley..." << std::endl;
    {
        pagmo::problem prob{pagmo::ackley(30)};
        benchmark_dist_solver_compare(address, prob);
    }

    std::cout << std::endl << "Running pagmo::griewank..." << std::endl;
    {
        pagmo::problem prob{pagmo::griewank(30)};
        benchmark_dist_solver_compare(address, prob);
    }
}

int main(int argc, char* argv[])
{
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal);

    if (argc >= 2 && argv[1] == std::string("-run-controller"))
    {
        const std::string address = "tcp://0.0.0.0:5000";
        udp_registry::get().set_local_cache_dir("controller_cache");

        run_benchmark_so(address);
    }
    else
    {
        const std::string address = "tcp://localhost:5000";
        distributed_worker worker{address, worker_mode::SINGLE_THREADED, 80};

        // Enable DLL fetching from controller
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
