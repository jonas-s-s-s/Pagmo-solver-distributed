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

        // Controller will try to assign worker identified by this ID to this island
        std::string _preferredWorkerId;

        static std::tuple<algorithm, population> _load_pagmo_pop_and_algo(const island& isl);

    public:
        void set_preferred_worker(const std::string& preferredWorkerId);

        void clear_preferred_worker();

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
