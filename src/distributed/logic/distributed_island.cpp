#include "distributed_island.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <pagmo/algorithm.hpp>
#include <pagmo/detail/gte_getter.hpp>
#include <pagmo/exceptions.hpp>
#include <pagmo/island.hpp>
#include <pagmo/population.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/threading.hpp>

#include "UUID.h"

namespace pagmo
{
    distributed_island::distributed_island() : _dealerSocket{_ctx}
    {
        _islandId = "island_" + uuid::v4::UUID::New().String();
    }

    // Island's name.
    std::string distributed_island::get_name() const
    {
        return "Distributed island";
    }

    // Island's extra info.
    std::string distributed_island::get_extra_info() const
    {
        return std::string("ID: ") + _islandId;
    }

    // Run evolve.
    void distributed_island::run_evolve(island& isl)
    {
        /*
        * 1) Define the work thread
        */

        auto impl = [&isl, this]()
        {
            std::cout << "Running distributed island" << std::endl;
            _dealerSocket.connect("ipc://distributed_controller_islands_socket");
            _dealerSocket.send(MsgType::ALLOCATE_WORK);

        };

        /*
        * 2) Execute the work thread
        */

        std::exception_ptr eptr;
        std::thread worker([&impl, &eptr]()
        {
            try
            {
                impl();
            }
            catch (...)
            {
                eptr = std::current_exception();
            }
        });

        worker.join();
        if (eptr)
        {
            std::rethrow_exception(eptr);
        }
    }

} // namespace pagmo