#pragma once

#include <string>

#include <pagmo/island.hpp>

#include "dealer_socket.h"

namespace pagmo
{
    class distributed_island
    {
        // We need to wrap there in shared ptr to make this class copy and move constructable
        // Or else it won't be recognized as UDI by Pagmo
        std::shared_ptr<zmq::context_t> _ctx;
        std::shared_ptr<distributed::dealer_socket> _dealerSocket;

        std::string _islandId;

        static std::tuple<algorithm, population> _load_pagmo_pop_and_algo(const island& isl);

    public:
        // Default ctor.
        distributed_island();

        // Island's name.
        std::string get_name() const;
        // Extra info.
        std::string get_extra_info() const;

        // run_evolve implementation.
        void run_evolve(island&) const;
    };
} // namespace pagmo
