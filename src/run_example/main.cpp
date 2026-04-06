#include "aixlog.hpp"
#include "distributed_controller.h"
#include "distributed_solver.h"
#include "distributed_worker.h"
#include "udp_dll_wrapper.h"
#include "udp_registry.h"
#include "pagmo/algorithms/de.hpp"
#include "pagmo/algorithms/gaco.hpp"


int main(int argc, char* argv[])
{
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);

    std::string address = "tcp://localhost:5000";

    if (argc >= 2 && argv[1] == std::string("-run-controller"))
    {
        udp_registry::get().set_local_cache_dir("controller_cache");

        unsigned param = 2;
        udp_dll_wrapper probWrapper{"schwefel_udp", std::any{param}};
        const pagmo::problem prob{probWrapper};

        pagmo::algorithm algo{pagmo::de(1500)};
        algo.set_verbosity(0);

        distributed_solver ds{address};
        ds.wait_until_workers_connect(2);

        ds.evolve(prob, {algo}, 1000);
        ds.wait_until_completion();

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        ds.evolve(prob, {algo}, 1000);
        ds.wait_until_completion();

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
    else
    {
        distributed_worker worker{address, worker_mode::ARCHIPELAGO_BASED};
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