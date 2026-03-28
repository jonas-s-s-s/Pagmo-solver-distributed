#include <utility>

#include "aixlog.hpp"
#include "distributed_controller.h"
#include "distributed_solver.h"
#include "distributed_worker.h"
#include "population_tools.h"
#include "solver_benchmark.h"
#include "udp_dll_wrapper.h"
#include "udp_registry.h"
#include "pagmo/algorithms/bee_colony.hpp"
#include "pagmo/algorithms/cmaes.hpp"
#include "pagmo/algorithms/de.hpp"
#include "pagmo/algorithms/de1220.hpp"
#include "pagmo/algorithms/gaco.hpp"
#include "pagmo/algorithms/gwo.hpp"
#include "pagmo/algorithms/pso.hpp"
#include "pagmo/algorithms/sade.hpp"
#include "pagmo/algorithms/simulated_annealing.hpp"
#include "pagmo/algorithms/xnes.hpp"
#include "pagmo/problems/rosenbrock.hpp"
#include "pagmo/problems/zdt.hpp"
#include "pagmo/utils/hypervolume.hpp"


void benchmark_single_objective_local()
{
    const pagmo::problem prob{pagmo::rosenbrock{10}}; // single-objective
    solver_benchmark s{prob};

    {
        const pagmo::population pop{prob, 100};
        const pagmo::de algo{50};
        s.start_timer();
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "First", s.stop_timer());
    }

    {
        const pagmo::population pop{prob, 60};
        const pagmo::de algo{50};
        s.start_timer();
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Second", s.stop_timer());
    }

    {
        const pagmo::population pop{prob, 40};
        const pagmo::de algo{50};
        s.start_timer();
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Third", s.stop_timer());
    }

    {
        const pagmo::population pop{prob, 40};
        const pagmo::de algo{30};
        s.start_timer();
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Fourth", s.stop_timer());
    }

    {
        const pagmo::population pop{prob, 40};
        const pagmo::de algo{20};
        s.start_timer();
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Fifth", s.stop_timer());
    }
    std::cout << s.get_benchmark_stats_csv() << std::endl;
}

void benchmark_multi_objective_local()
{
    const pagmo::problem prob{pagmo::zdt{1}};
    solver_benchmark s{prob};

    {
        const pagmo::population pop{prob, 60};
        const pagmo::nsga2 algo{50};
        s.start_timer();
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "First", s.stop_timer());
    }

    {
        const pagmo::population pop{prob, 60};
        const pagmo::nsga2 algo{50};
        s.start_timer();
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Second", s.stop_timer());
    }

    {
        const pagmo::population pop{prob, 40};
        const pagmo::nsga2 algo{50};
        s.start_timer();
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Third", s.stop_timer());
    }

    {
        const pagmo::population pop{prob, 40};
        const pagmo::nsga2 algo{30};
        s.start_timer();
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Fourth", s.stop_timer());
    }

    {
        const pagmo::population pop{prob, 40};
        const pagmo::nsga2 algo{20};
        s.start_timer();
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Fifth", s.stop_timer());
    }

    std::cout << s.get_benchmark_stats_csv() << std::endl;
}

void benchmark_distributed_solver_controller_GACO(std::string address)
{
    udp_registry::get().set_local_cache_dir("controller_cache");

    udp_dll_wrapper probWrapper{"schwefel_udp"};
    const pagmo::problem prob{probWrapper};
    solver_benchmark s{prob};

    distributed_solver ds{address};
    ds.wait_until_workers_connect(1);

    // Problem: Schwefel, Algorith: GACO, Generations: 1500, PopSize: 100
    {
        pagmo::algorithm algo{pagmo::gaco(1500)};
        algo.set_verbosity(0);
        s.start_timer();
        ds.evolve(prob, {algo}, 100);
        ds.wait_until_completion();
        s.add_result(ds.get_best_individual(), "First", s.stop_timer());
    }

    {
        pagmo::algorithm algo{pagmo::gaco(1250)};
        algo.set_verbosity(0);
        s.start_timer();
        ds.evolve(prob, {algo}, 75);
        ds.wait_until_completion();
        s.add_result(ds.get_best_individual(), "Second", s.stop_timer());
    }

    {
        pagmo::algorithm algo{pagmo::gaco(750)};
        algo.set_verbosity(0);
        s.start_timer();
        ds.evolve(prob, {algo}, 75);
        ds.wait_until_completion();
        s.add_result(ds.get_best_individual(), "Third", s.stop_timer());
    }

    {
        pagmo::algorithm algo{pagmo::gaco(500)};
        algo.set_verbosity(0);
        s.start_timer();
        ds.evolve(prob, {algo}, 75);
        ds.wait_until_completion();
        s.add_result(ds.get_best_individual(), "Fourth", s.stop_timer());
    }

    {
        pagmo::algorithm algo{pagmo::gaco(250)};
        algo.set_verbosity(0);
        s.start_timer();
        ds.evolve(prob, {algo}, 75);
        ds.wait_until_completion();
        s.add_result(ds.get_best_individual(), "Fifth", s.stop_timer());
    }

    std::cout << s.get_benchmark_stats_csv() << std::endl;
}

void benchmark_distributed_solver_controller_META(std::string address)
{
    udp_registry::get().set_local_cache_dir("controller_cache");

    udp_dll_wrapper probWrapper{"schwefel_udp"};
    const pagmo::problem prob{probWrapper};
    solver_benchmark s{prob};

    distributed_solver ds{address};
    ds.wait_until_workers_connect(8);

    //Problem: Schwefel, Algorithm: GACO, Generations: 1500, PopSize: 100
    {
        pagmo::algorithm algo{pagmo::gaco(1500)};
        algo.set_verbosity(0);
        s.start_timer();
        ds.evolve(prob, {algo}, 100);
        ds.wait_until_completion();
        s.add_result(ds.get_best_individual(), "GACO", s.stop_timer());
    }

    //Problem: Schwefel, Algorithm: META, Generations: 1500, PopSize: 100
    {
        pagmo::algorithm a1{pagmo::gaco(1500)};
        pagmo::algorithm a2{pagmo::de(1500)};
        pagmo::algorithm a3{pagmo::sade(1500)};
        pagmo::algorithm a4{pagmo::de1220(1500)};
        pagmo::algorithm a5{pagmo::gwo(1500)};
        pagmo::algorithm a6{pagmo::pso(1500)};
        pagmo::algorithm a7{pagmo::bee_colony(1500)};
        pagmo::algorithm a8{pagmo::simulated_annealing(1500)};

        s.start_timer();
        ds.evolve(prob, {a1, a2, a3, a4, a5, a6, a7, a8}, 100);
        ds.wait_until_completion();
        s.add_result(ds.get_best_individual(), "META", s.stop_timer());
    }

    std::cout << s.get_benchmark_stats_csv() << std::endl;
}

void benchmark_distributed_solver(int argc, char** argv)
{
    std::string address = "tcp://localhost:5000";

    if (argc >= 2 && argv[1] == std::string("-run-controller"))
    {
        //benchmark_distributed_solver_controller_GACO(address);
        benchmark_distributed_solver_controller_META(address);
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

        worker.run_client();
    }
}

int main(int argc, char* argv[])
{
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);

    benchmark_distributed_solver(argc, argv);

    //benchmark_multi_objective_local();

    //benchmark_single_objective_local();

    return 0;
}
