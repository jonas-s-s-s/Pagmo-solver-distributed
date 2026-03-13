#include <fstream>

#include "distributed_controller.h"
#include "distributed_worker.h"
#include "islandTest.h"
#include "udp_dll_wrapper.h"
#include "udp_registry.h"
#include "pagmo/problems/zdt.hpp"


int main(int argc, char* argv[])
{
    dll_locator locator;
    udp_registry::get().set_lib_cache("controller_cache");
    udp_registry::get().register_udp_provider(
        [&locator](const std::string& libName)
        {
            return locator.get_dll(libName);
        }
    );

    pagmo::algorithm nsga2{pagmo::nsga2(100)};
    udp_dll_wrapper probWrapper{"problems"};

    const pagmo::problem prob{probWrapper};
    pagmo::population pop{prob};

    const work_container wcr{nsga2, pop};;

    std::vector<std::byte> serialized;
    vector_streambuf buf(serialized);
    std::ostream os(&buf);

    boost::archive::binary_oarchive oa(os);
    oa << wcr;
    os.flush();

    /*
    std::string address = "tcp://localhost:5000";
    std::thread t;

    if (argc >= 2 && argv[1] == std::string("-run-controller"))
    {
        // TODO: This locator is possibly duplicated, we already define a locator inside of controller
        dll_locator locator;
        udp_registry::get().set_lib_cache("controller_cache");
        udp_registry::get().register_udp_provider(
            [&locator](const std::string& libName)
            {
                return locator.get_dll(libName);
            }
        );

        distributed_controller controller{address};
        controller.run_server();

        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        islandTest::run_gaco(islandTest::run_dll_problem);

        //islandTest::run_meta_multiobjective(islandTest::run_zdt);
    }
    else
    {
        distributed_worker worker{address};
        // We register this worker with the UDP registry, so DLLs can be requested from controller
        udp_registry::get().set_lib_cache("worker_cache");
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
    */

    return 0;
}
