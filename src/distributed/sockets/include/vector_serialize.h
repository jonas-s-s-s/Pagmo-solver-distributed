#pragma once
#include <vector>

#include "vector_streambuf.h"
#include <pagmo/s11n.hpp>
#include <boost/serialization/optional.hpp>

/**
 * Any part of the application can use this to serialize an object into byte vector.
 * Defined only here, any changes will affect serialization of all sockets.
 */
template <typename Serializable>
static std::vector<std::byte> vector_serialize(const Serializable& payload)
{
    std::vector<std::byte> serialized;
    vector_streambuf buf(serialized);
    std::ostream os(&buf);

    // TODO: binary_oarchive is not portable??
    boost::archive::binary_oarchive oa(os);
    oa << payload;
    os.flush();

    return serialized;
}