#include "algorithms/include/base_problem.h"
#include "distributed_controller.h"
#include "distributed_island.h"
#include "distributed_worker.h"
#include "algorithms/include/dll_problem_wrapper.h"
#include "islandTest.h"
#include "lib_loader.h"
#include "vector_istreambuf.h"
#include "pagmo/algorithms/gaco.hpp"
#include "pagmo/algorithms/nsga2.hpp"
#include "pagmo/problems/dtlz.hpp"
#include <boost/serialization/shared_ptr.hpp>


int main(int argc, char* argv[])
{
    lib_loader<base_problem> ll{"./problems" + portable_dll_extension()};
    ll.open_lib();
    std::shared_ptr<base_problem> bp = ll.get_instance();
    dll_problem_wrapper dpw{bp, bp->get_lib_file_name()};

    std::cout << bp->get_lib_file_name() << std::endl;

    std::cout << typeid(*bp).name() << std::endl;


    std::vector<std::byte> serialized;
    vector_streambuf buf(serialized);
    std::ostream os(&buf);

    boost::archive::binary_oarchive oa(os);
    oa << dpw;
    os.flush();

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

    // ZDT test suite is used in the NSGA II paper as well
    //islandTest::run_nsga2(islandTest::run_zdt);

    return 0;
}
