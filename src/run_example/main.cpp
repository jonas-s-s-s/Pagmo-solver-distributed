#include <utility>

#include "aixlog.hpp"
#include "distributed_controller.h"
#include "distributed_solver.h"
#include "distributed_worker.h"
#include "population_tools.h"
#include "solver_benchmark.h"
#include "udp_dll_wrapper.h"
#include "udp_registry.h"
#include "pagmo/algorithms/de.hpp"
#include "pagmo/algorithms/gaco.hpp"
#include "pagmo/problems/rosenbrock.hpp"
#include "pagmo/problems/zdt.hpp"
#include "pagmo/utils/hypervolume.hpp"


int main(int argc, char* argv[])
{
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);

    /*
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

    {
        const pagmo::population pop{prob, 40};
        const pagmo::nsga2 algo{30};
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Fourth");
    }

    {
        const pagmo::population pop{prob, 40};
        const pagmo::nsga2 algo{20};
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Fifth");
    }

    std::cout << s.get_benchmark_stats_csv() << std::endl;
    */


    const pagmo::problem prob{pagmo::rosenbrock{10}}; // single-objective
    solver_benchmark s{prob};

    {
        const pagmo::population pop{prob, 100};
        const pagmo::de algo{50};
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "First");
    }

    {
        const pagmo::population pop{prob, 60};
        const pagmo::de algo{50};
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Second");
    }

    {
        const pagmo::population pop{prob, 40};
        const pagmo::de algo{50};
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Third");
    }

    {
        const pagmo::population pop{prob, 40};
        const pagmo::de algo{30};
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Fourth");
    }

    {
        const pagmo::population pop{prob, 40};
        const pagmo::de algo{20};
        const auto out = algo.evolve(pop);
        const auto best = select_best_individual(prob, out.get_x(), out.get_f());
        s.add_result(best, "Fifth");
    }
    std::cout << s.get_benchmark_stats_csv() << std::endl;

    return 0;
}
