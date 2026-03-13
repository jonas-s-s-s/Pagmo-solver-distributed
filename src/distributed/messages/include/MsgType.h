#pragma once

#include "pagmo/algorithm.hpp"
#include "pagmo/population.hpp"
#include <pagmo/s11n.hpp>
#include <pagmo/detail/s11n_wrappers.hpp>
#include <utility>
#include <optional>

#include "pagmo/algorithms/nsga2.hpp"

enum class MsgType
{
    WORKER_JOIN,
    WORKER_LEAVE,
    ALLOCATE_WORK,
    WORK_RESULTS,
    GET_DLL,
    DLL_BINARY
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


class get_dll_request
{
public:
    std::string dll_name;
    // TODO: Remove this, we can use constant sender IDs for our sockets in Worker
    std::string sender_id;

    get_dll_request(const std::string& dll_name, const std::string& sender_id)
        : dll_name(dll_name),
          sender_id(sender_id)
    {
    }

    get_dll_request() = default;

private:
    //####################################
    //# BOOST SERIALIZE
    //####################################

    friend class boost::serialization::access;

    template <typename Archive>
    void save(Archive& ar, unsigned) const
    {
        pagmo::detail::to_archive(ar, dll_name,sender_id);
    }

    template <typename Archive>
    void load(Archive& ar, unsigned)
    {
        pagmo::detail::from_archive(ar, dll_name,sender_id);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

class dll_binary_container
{
public:
    std::string dll_name;
    std::optional<std::vector<std::byte>> dll_file;
    // TODO: Remove this, we can use constant sender IDs for our sockets in Worker
    std::string sender_id;

    dll_binary_container(const std::string& dll_name, const std::optional<std::vector<std::byte>>& dll_file,
        const std::string& sender_id)
        : dll_name(dll_name),
          dll_file(dll_file),
          sender_id(sender_id)
    {
    }

    dll_binary_container() = default;

private:
    //####################################
    //# BOOST SERIALIZE
    //####################################

    friend class boost::serialization::access;

    template <typename Archive>
    void save(Archive& ar, unsigned) const
    {
        pagmo::detail::to_archive(ar, dll_name, dll_file, sender_id);
    }

    template <typename Archive>
    void load(Archive& ar, unsigned)
    {
        pagmo::detail::from_archive(ar, dll_name, dll_file, sender_id);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};
