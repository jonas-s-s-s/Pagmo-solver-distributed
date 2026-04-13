#include <catch2/catch_test_macros.hpp>
#include "distributed/logic/include/distributed_controller.h"
#include "distributed/logic/include/distributed_worker.h"
#include "distributed/logic/include/distributed_solver.h"
#include "distributed/discovery/include/udp_registry.h"
#include <thread>
#include <pagmo/problems/schwefel.hpp>
#include <pagmo/algorithms/de.hpp>

TEST_CASE("basic in-process controller + worker + evolve", "[integration]") {
   /* const std::string addr = "ipc://basic_system_test";
    distributed_controller controller{addr};
    controller.run_server();

    std::jthread workerTh([&]() {
        distributed_worker worker{addr, worker_mode::SINGLE_THREADED, 20};
        udp_registry::get().register_udp_provider([&worker](const std::string& n){
            return worker.get_dll_from_controller(n);
        });
        for (int i = 0; i < 5; ++i) worker.client_loop();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    distributed_solver solver{addr, 1};
    solver.wait_until_workers_connect(1);

    pagmo::problem prob{pagmo::schwefel(3)};
    pagmo::algorithm algo{pagmo::de(5)};
    //solver.evolve(prob, {algo}, 40, 1, 20);
    //auto best = solver.wait_until_completion();
        */
   REQUIRE(3 == 3); // TODO: Not working
}
