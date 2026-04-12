#include "aixlog.hpp"
#include "distributed_controller.h"
#include "distributed_solver.h"
#include "distributed_worker.h"
#include "udp_dll_wrapper.h"
#include "udp_registry.h"
#include "pagmo/algorithms/de.hpp"
#include "pagmo/algorithms/gaco.hpp"

#include <iostream>
#include <string>
#include <stdexcept>

#include "benchmark_stats.h"
#include "so_benchmark.h"

// ####################################################
// # DIFFERENT CONTROLLER TASKS
// ####################################################

void benchmark_controller_task(const std::string& address, const size_t expectedWorkerCount,
                               const load_balancing_strategy loadBalancingStrategy, const std::string& localCacheDir)
{
    udp_registry::get().set_local_cache_dir(localCacheDir);

    distributed_solver ds{address, expectedWorkerCount, loadBalancingStrategy};
    benchmark_stats bench{};
    run_so_benchmark(ds, bench);
}

void default_controller_task(const std::string& address, const size_t expectedWorkerCount,
                             const load_balancing_strategy loadBalancingStrategy, const std::string& localCacheDir)
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

// ####################################################
// # CONTROLLER / WORKER RUN
// ####################################################

void run_controller(const std::string& address, const size_t expectedWorkerCount,
                    const load_balancing_strategy loadBalancingStrategy, const std::string& localCacheDir,
                    const std::function<void(std::string, size_t, load_balancing_strategy, std::string)>& runFunc)
{
    std::string errMsg = "Aborting distributed solver, an exception occurred: ";
    try
    {
        runFunc(address, expectedWorkerCount, loadBalancingStrategy, localCacheDir);
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

void run_worker(const std::string& address, worker_mode mode,
                size_t minIslandPopSize, const std::string& localCacheDir)
{
    std::string errMsg = "Aborting distributed worker, an exception occurred: ";
    try
    {
        distributed_worker worker{address, mode, minIslandPopSize};
        udp_registry::get().set_local_cache_dir(localCacheDir);
        udp_registry::get().register_udp_provider(
            [&worker](const std::string& libName) { return worker.get_dll_from_controller(libName); });
        for (;;) worker.client_loop();
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

// ####################################################
// # PARSING
// ####################################################

void print_help(char** argv)
{
    std::cout <<
        "Usage:\n"
        "  " << argv[0] << " --controller [options]\n"
        "  " << argv[0] << " --worker     [options]\n"
        "\n"
        "Controller options:\n"
        "  --address      <addr>  (default: tcp://0.0.0.0:5000)\n"
        "  --workers      <n>     Expected worker count (default: 2)\n"
        "  --strategy     <name>  (default: BY_PERFORMANCE)\n"
        "  --cache-dir    <dir>   (default: controller_cache)\n"
        "\n"
        "Worker options:\n"
        "  --address      <addr>  (default: tcp://localhost:5000)\n"
        "  --mode         <name>  (default: SINGLE_THREADED)\n"
        "  --min-pop-size <n>     (default: 80)\n"
        "  --cache-dir    <dir>   (default: worker_cache)"
        << std::endl;
}

struct main_args
{
    bool isController = false;

    std::string address = ""; // Set by user
    std::string defaultAddressWorker = "tcp://localhost:5000";
    std::string defaultAddressController = "tcp://localhost:5000";
    size_t workers = 2;

    std::string strategy = "BY_PERFORMANCE";
    std::string workerMode = "SINGLE_THREADED";
    size_t minPopSize = 80;

    std::string cacheDir = ""; // Set by user
    std::string defaultCacheDirWorker = "worker_cache";
    std::string defaultCacheDirController = "controller_cache";

    bool runBenchmark = false;
    bool disableLogging = false;
};

std::optional<main_args> parse_main_args(const int argc, char** argv)
{
    main_args mainArgs{};
    bool modeSet = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        auto nextArg = [&]() -> std::string
        {
            if (++i >= argc)
                throw std::runtime_error(a + " requires a value");
            return argv[i];
        };

        if (a == "--help" || a == "-h")
        {
            // handled in main
        }
        else if (a == "--controller")
        {
            mainArgs.isController = true;
            modeSet = true;
        }
        else if (a == "--benchmark")
        {
            mainArgs.runBenchmark = true;
        }
        else if (a == "--disable-logging")
        {
            mainArgs.disableLogging = true;
        }
        else if (a == "--worker")
        {
            mainArgs.isController = false;
            modeSet = true;
        }
        else if (a == "--address")
        {
            mainArgs.address = nextArg();
        }
        else if (a == "--workers")
        {
            mainArgs.workers = std::stoul(nextArg());
        }
        else if (a == "--strategy")
        {
            mainArgs.strategy = nextArg();
        }
        else if (a == "--mode")
        {
            mainArgs.workerMode = nextArg();
        }
        else if (a == "--min-pop-size")
        {
            mainArgs.minPopSize = std::stoul(nextArg());
        }
        else if (a == "--cache-dir")
        {
            mainArgs.cacheDir = nextArg();
        }
        else
        {
            std::cerr << "Unknown argument: " << a << "\n";
            modeSet = false;
            break;
        }
    }

    if (!modeSet)
    {
        return std::nullopt;
    }
    return mainArgs;
}

// ####################################################
// # MAIN
// ####################################################
int main(int argc, char* argv[])
{
    // Parse cmd line args
    const auto& mainArgsOpt = parse_main_args(argc, argv);
    if (!mainArgsOpt.has_value())
    {
        print_help(argv);
        return 2;
    }
    const auto& ma = mainArgsOpt.value();

    if (ma.disableLogging)
    {
        AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal);
    }
    else
    {
        AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);
    }

    // Run controller / worker
    try
    {
        if (ma.isController)
        {
            const auto controllerTask = (ma.runBenchmark) ? benchmark_controller_task : default_controller_task;

            const load_balancing_strategy lbs = (ma.strategy == "ALL_ISLANDS_EQUAL")
                                                    ? load_balancing_strategy::ALL_ISLANDS_EQUAL
                                                    : load_balancing_strategy::BY_PERFORMANCE;

            run_controller(
                ma.address.empty() ? ma.defaultAddressController : ma.address,
                ma.workers,
                lbs,
                ma.cacheDir.empty() ? ma.defaultCacheDirController : ma.cacheDir,
                controllerTask
            );
        }
        else
        {
            const worker_mode wm = (ma.workerMode == "ARCHIPELAGO_BASED")
                                       ? worker_mode::ARCHIPELAGO_BASED
                                       : worker_mode::SINGLE_THREADED;

            run_worker(
                ma.address.empty() ? ma.defaultAddressWorker : ma.address,
                wm,
                ma.minPopSize,
                ma.cacheDir.empty() ? ma.defaultCacheDirWorker : ma.cacheDir
            );
        }
    }
    catch (const std::exception& e)
    {
        return 2;
    }

    return 0;
}
