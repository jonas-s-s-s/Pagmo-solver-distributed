#pragma once

#include "pagmo/algorithm.hpp"
#include "pagmo/population.hpp"
#include <pagmo/s11n.hpp>
#include <pagmo/detail/s11n_wrappers.hpp>
#include <utility>

#include "pagmo/algorithms/nsga2.hpp"

enum class MsgType
{
    WORKER_JOIN,
    WORKER_LEAVE,
    ALLOCATE_WORK,
    WORK_RESULTS,
};

class work_container
{
public:
    pagmo::algorithm algo;
    pagmo::population pop;

    work_container(pagmo::algorithm algo, pagmo::population pop) : algo(std::move(algo)), pop(std::move(pop))
    {
    }

    work_container() = default;

private:
    //####################################
    //# BOOST SERIALIZE
    //####################################

    friend class boost::serialization::access;

    template <typename Archive>
    void save(Archive& ar, unsigned) const
    {
        pagmo::detail::to_archive(ar, algo, pop);
    }

    template <typename Archive>
    void load(Archive& ar, unsigned)
    {
        try
        {
            pagmo::detail::from_archive(ar, algo, pop);
        }
        catch (...)
        {
            *this = work_container{};
            throw;
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};
