#pragma once

#include <string>

#include <pagmo/island.hpp>

namespace pagmo
{

    class distributed_island
    {
    public:
        // Default ctor.
        distributed_island();

        // Island's name.
        std::string get_name() const;
        // Extra info.
        std::string get_extra_info() const;

        // run_evolve implementation.
        void run_evolve(island &) const;
    };

} // namespace pagmo