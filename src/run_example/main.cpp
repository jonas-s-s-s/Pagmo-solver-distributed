#include <fstream>

#include "distributed_controller.h"
#include "distributed_worker.h"
#include "islandTest.h"
#include "udp_registry.h"

std::optional<std::vector<std::byte>> lib_provider(const std::string& lib_name)
{
    // TODO: remove ".dll"
    try
    {
        std::basic_ifstream<std::byte> fStream{lib_name + ".dll", std::ios::binary};
        std::vector<std::byte> file_content{std::istreambuf_iterator(fStream), {}};
        return file_content;
    } catch (...)
    {
        return std::nullopt;
    }
}

int main(int argc, char* argv[])
{
    udp_registry::get().register_udp_provider(lib_provider);

    const auto udp = udp_registry::get().construct_udp("problems");

    std::cout << udp->get_lib_file_name() << std::endl;


    /*
    std::string address = "tcp://localhost:5000";
    std::thread t;

    if (argc >= 2 && argv[1] == std::string("-run-controller"))
    {
        distributed_controller controller{address};
        controller.run_server();

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        islandTest::run_gaco(islandTest::run_dll_problem);

        //islandTest::run_meta_multiobjective(islandTest::run_zdt);
    }
    else
    {
        distributed_worker worker{address};
        for (;;)
        {
            worker.client_loop();
        }
    }
    t.join();
    */

    return 0;
}
