+
#include "aixlog.hpp"
#include "distributed_solver.h"
#include "distributed_worker.h"
#include "benchmark_stats.h"
#include "udp_registry.h"
#include "so_benchmark.h"

void run_controller(const std::string& address, size_t expectedWorkerCount,
                    load_balancing_strategy loadBalancingStrategy, const std::string& localCacheDir)
{
    std::string errMsg = "Aborting distributed solver benchmark, an excpetion occured: ";
    try
    {
        udp_registry::get().set_local_cache_dir(localCacheDir);

        distributed_solver ds{address, expectedWorkerCount, loadBalancingStrategy};
        benchmark_stats bench{};
        run_so_benchmark(ds, bench);
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

void run_worker(const std::string& address, worker_mode mode, const size_t minIslandPopSize,
                const std::string& localCacheDir)
{
    std::string errMsg = "Aborting distributed worker, an excpetion occured: ";
    try
    {
        distributed_worker worker{address, mode, minIslandPopSize};

        // Enable DLL fetching from controller
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
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal);

    if (argc >= 2 && argv[1] == std::string("-run-controller"))
    {
        run_controller("tcp://0.0.0.0:5000", 8, load_balancing_strategy::ALL_ISLANDS_EQUAL, "controller_cache");
    }
    else
    {
        run_worker("tcp://localhost:5000", worker_mode::SINGLE_THREADED, 80, "worker_cache");
    }

    return 0;
}
