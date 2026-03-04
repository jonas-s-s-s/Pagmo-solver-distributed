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
#include "pagmo/utils/multi_objective.hpp"

namespace pagmo
{
    std::tuple<algorithm, population> distributed_island::_load_pagmo_pop_and_algo(const island& isl)
    {
        // Load algorithm and population from the island object in exactly the same way as pagmo::thread_island

        algorithm algo;
        population pop;

        const auto gte = detail::gte_getter();
        (void)gte;

        const auto tmp_algo(isl.get_algorithm());
        auto tmp_pop(isl.get_population());

        if (tmp_algo.get_thread_safety() < thread_safety::basic)
        {
            pagmo_throw(std::invalid_argument,
                        "the 'distributed_island' UDI requires an algorithm providing at least the 'basic' "
                        "thread safety guarantee, but an algorithm of type '"
                        + tmp_algo.get_name() + "' does not");
        }

        if (tmp_pop.get_problem().get_thread_safety() < thread_safety::basic)
        {
            pagmo_throw(std::invalid_argument,
                        "the 'distributed_island' UDI requires a problem providing at least the 'basic' "
                        "thread safety guarantee, but a problem of type '"
                        + tmp_pop.get_problem().get_name() + "' does not");
        }

        return {algo, pop};
    }

    distributed_island::distributed_island() : _ctx{new zmq::context_t{}}, _dealerSocket{new distributed::dealer_socket(*_ctx)}
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
    void distributed_island::run_evolve(island& isl) const
    {
        /*
        * 1) Define the work thread
        */

        auto impl = [&isl, this]()
        {
            // 1) Load pop and algo
            auto [algo, pop] = _load_pagmo_pop_and_algo(isl);

            // 2) Allocate our algorithm and population to some worker node via controller
            _dealerSocket->set_routing_id(_islandId);
            std::cout << "Running distributed island" << std::endl;
            _dealerSocket->connect("ipc://distributed_controller_islands_socket");
            std::cout << "Distributed island connected" << std::endl;
            _dealerSocket->send(MsgType::ALLOCATE_WORK);
            std::cout << "Distributed island allocate work sent" << std::endl;

            // 3) Wait until controller returns results from worker
            auto [type, binary] = _dealerSocket->receive();
            std::cout << "island received [" << static_cast<int>(type) << "] from controller" << std::endl;

            // 4) Pass results back to the pagmo::island object
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
