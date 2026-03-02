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

namespace pagmo
{

distributed_island::distributed_island() : distributed_island(true) {}

distributed_island::distributed_island(bool use_pool) : m_use_pool(use_pool) {}

// Island's name.
std::string distributed_island::get_name() const
{
    return "Distributed island";
}

// Island's extra info.
std::string distributed_island::get_extra_info() const
{
    return std::string("\tUsing pool: ") + (m_use_pool ? "yes" : "no");
}

// Run evolve.
void distributed_island::run_evolve(island &isl) const
{
    auto impl = [&isl]() {
        // Init default-constructed algo/pop. We will move
        // later into these variables the algo/pop from isl.
        algorithm algo;
        population pop;

        {
            // NOTE: run_evolve() is called from the separate
            // thread of execution within pagmo::island. Since
            // we need to extract copies of algorithm and population,
            // which may be implemented in Python, we need to protect
            // with a gte.
            auto gte = detail::gte_getter();
            (void)gte;

            // Get copies of algo/pop from isl.
            // NOTE: in case of exceptions, any pythonic object
            // existing within this scope will be destroyed before the gte,
            // while it is still safe to call into Python.
            auto tmp_algo(isl.get_algorithm());
            auto tmp_pop(isl.get_population());

            // Check the thread safety levels.
            if (tmp_algo.get_thread_safety() < thread_safety::basic) {
                pagmo_throw(std::invalid_argument,
                            "the 'distributed_island' UDI requires an algorithm providing at least the 'basic' "
                            "thread safety guarantee, but an algorithm of type '"
                                + tmp_algo.get_name() + "' does not");
            }

            if (tmp_pop.get_problem().get_thread_safety() < thread_safety::basic) {
                pagmo_throw(std::invalid_argument,
                            "the 'distributed_island' UDI requires a problem providing at least the 'basic' "
                            "thread safety guarantee, but a problem of type '"
                                + tmp_pop.get_problem().get_name() + "' does not");
            }

            // Move the copies into algo/pop. At this point, we know
            // that algo and pop are not pythonic, as pythonic entities are never
            // marked as thread-safe.
            algo = std::move(tmp_algo);
            pop = std::move(tmp_pop);
        }

        // Evolve and replace the island's population with the evolved population.
        isl.set_population(algo.evolve(pop));
        // Replace the island's algorithm with the algorithm used for the evolution.
        // NOTE: if set_algorithm() fails, we will have the new population with the
        // original algorithm, which is still a valid state for the island.
        isl.set_algorithm(algo);
    };

    if (m_use_pool) {
        // Spawn a single std::thread
        // Manual exception transport, as with the original code
        std::exception_ptr eptr;

        std::thread worker([&impl, &eptr]() {
            try {
                impl();
            } catch (...) {
                eptr = std::current_exception();
            }
        });

        worker.join();

        if (eptr) {
            std::rethrow_exception(eptr);
        }
    } else {
        impl();
    }
}

// Serialization support.
template <typename Archive>
void distributed_island::save(Archive &ar, unsigned) const
{
    ar << m_use_pool;
}

template <typename Archive>
void distributed_island::load(Archive &ar, unsigned version)
{
    if (version > 0u) {
        ar >> m_use_pool;
    } else {
        // LCOV_EXCL_START
        // NOTE: if loading from version 0,
        // set the flag to false (as the version 0
        // of distributed_island had no support for
        // a thread pool).
        m_use_pool = false;
        // LCOV_EXCL_STOP
    }
}

} // namespace pagmo

PAGMO_S11N_ISLAND_IMPLEMENT(pagmo::distributed_island)
