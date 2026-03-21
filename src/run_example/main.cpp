#include <fstream>

#include "distributed_controller.h"
#include "distributed_worker.h"
#include "islandTest.h"
#include "udp_dll_wrapper.h"
#include "udp_registry.h"
#include "pagmo/problems/zdt.hpp"


int main(int argc, char* argv[])
{
    std::string address = "tcp://localhost:5000";
    std::thread t;

    if (argc >= 2 && argv[1] == std::string("-run-controller"))
    {
        udp_registry::get().set_local_cache_dir("controller_cache");

        distributed_controller controller{address};
        controller.run_server();

        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        islandTest::run_gaco(islandTest::run_dll_problem);

        //islandTest::run_meta_multiobjective(islandTest::run_zdt);
    }
    else
    {
        distributed_worker worker{address};

        std::mt19937 rng{std::random_device{}()};
        int tempWorkerId = std::uniform_int_distribution<int>{1, 100}(rng);

        // We register this worker with the UDP registry, so DLLs can be requested from controller
        udp_registry::get().set_local_cache_dir("worker_cache" + std::to_string(tempWorkerId));
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
    t.join();

    return 0;
}
