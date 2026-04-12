#include "aixlog.hpp"
#include "distributed_controller.h"
#include "distributed_solver.h"
#include "distributed_worker.h"
#include "udp_dll_wrapper.h"
#include "udp_registry.h"
#include "pagmo/algorithms/de.hpp"
#include "pagmo/algorithms/gaco.hpp"


void run_controller(const std::string& address, size_t expectedWorkerCount,
                    load_balancing_strategy loadBalancingStrategy, const std::string& localCacheDir)
{
    std::string errMsg = "Aborting distributed solver, an excpetion occured: ";
    try
    {
        udp_registry::get().set_local_cache_dir(localCacheDir);

        unsigned param = 2;
        udp_dll_wrapper probWrapper{"schwefel_udp", std::any{param}};
        const pagmo::problem prob{probWrapper};

        pagmo::algorithm algo{pagmo::de(1500)};
        algo.set_verbosity(0);

        distributed_solver ds{address, expectedWorkerCount, loadBalancingStrategy};
        ds.wait_until_workers_connect();

        ds.evolve(prob, {algo}, 1000);
        ds.wait_until_completion();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        ds.evolve(prob, {algo}, 1000);
        ds.wait_until_completion();

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
    catch (const std::exception& e)
    {
        errMsg += e.what();
        LOG(FATAL) << errMsg;
        throw std::runtime_error(errMsg);
    }
    catch (const std::string& e)
    {
        errMsg += e;
        LOG(FATAL) << errMsg;
        throw std::runtime_error(errMsg);
    }
    catch (const char* e)
    {
        errMsg += e;
        LOG(FATAL) << errMsg;
        throw std::runtime_error(errMsg);
    }
    catch (...)
    {
        errMsg += "unknown exception type";
        LOG(FATAL) << errMsg;
        throw std::runtime_error(errMsg);
    }
}

void run_worker(const std::string& address, worker_mode mode, const size_t minIslandPopSize, const std::string& localCacheDir)
{
    std::string errMsg = "Aborting distributed worker, an excpetion occured: ";
    try
    {

        distributed_worker worker{address, mode, minIslandPopSize};
        // We register this worker with the UDP registry, so DLLs can be requested from controller
        udp_registry::get().set_local_cache_dir(localCacheDir);
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
    catch (const std::exception& e)
    {
        errMsg += e.what();
        LOG(FATAL) << errMsg;
        throw std::runtime_error(errMsg);
    }
    catch (const std::string& e)
    {
        errMsg += e;
        LOG(FATAL) << errMsg;
        throw std::runtime_error(errMsg);
    }
    catch (const char* e)
    {
        errMsg += e;
        LOG(FATAL) << errMsg;
        throw std::runtime_error(errMsg);
    }
    catch (...)
    {
        errMsg += "unknown exception type";
        LOG(FATAL) << errMsg;
        throw std::runtime_error(errMsg);
    }
}

int main(int argc, char* argv[])
{
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);

    if (argc >= 2 && argv[1] == std::string("-run-controller"))
    {
        run_controller("tcp://0.0.0.0:5000", 2, load_balancing_strategy::BY_PERFORMANCE, "controller_cache");
    }
    else
    {
        run_worker("tcp://localhost:5000", worker_mode::SINGLE_THREADED, 80, "worker_cache");
    }
    return 0;
}
