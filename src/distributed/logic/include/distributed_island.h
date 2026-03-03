#pragma once

#include <string>

#include <pagmo/island.hpp>

#include "dealer_socket.h"

namespace pagmo
{

    class distributed_island
    {
        zmq::context_t _ctx;
        distributed::dealer_socket _dealerSocket;
        std::string _islandId;

    public:
        // Default ctor.
        distributed_island();

        // Island's name.
        std::string get_name() const;
        // Extra info.
        std::string get_extra_info() const;

        // run_evolve implementation.
        void run_evolve(island &);
    };

} // namespace pagmo