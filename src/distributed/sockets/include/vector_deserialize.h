#pragma once
#include <vector>

#include "vector_istreambuf.h"
#include <pagmo/s11n.hpp>
#include <boost/serialization/optional.hpp>

/**
 * Deserialize an object from byte vector produced by vector_serialize().
 * Any part of the application can use this to abstract boost-specific code.
 */
template <typename Serializable>
Serializable vector_deserialize(const std::vector<std::byte>& binary)
{
    if (binary.empty())
        throw std::runtime_error("Error: Attempted to deserialize an empty buffer");

    vector_istreambuf buf(binary);
    std::istream is(&buf);

    // Changed from binary archive to achieve better compatibility
    boost::archive::text_iarchive ia(is);
    Serializable obj{};
    ia >> obj;

    return obj;
}
