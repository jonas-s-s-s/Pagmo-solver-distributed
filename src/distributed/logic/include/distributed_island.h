#pragma once

#include <string>

#include <pagmo/detail/visibility.hpp>
#include <pagmo/island.hpp>
#include <pagmo/s11n.hpp>

namespace pagmo
{

    class distributed_island
    {
    public:
        // Default ctor.
        distributed_island();
        // Ctor with use_pool flag.
        explicit distributed_island(bool);

        // Island's name.
        std::string get_name() const;
        // Extra info.
        std::string get_extra_info() const;

        // run_evolve implementation.
        void run_evolve(island &) const;

    private:
        // Object serialization
        friend class boost::serialization::access;
        template <typename Archive>
        void save(Archive &, unsigned) const;
        template <typename Archive>
        void load(Archive &, unsigned);
        BOOST_SERIALIZATION_SPLIT_MEMBER()

        bool m_use_pool;
    };

} // namespace pagmo

PAGMO_S11N_ISLAND_EXPORT_KEY(pagmo::distributed_island)

// NOTE: version 1 added the m_use_pool flag.
BOOST_CLASS_VERSION(pagmo::distributed_island, 1)