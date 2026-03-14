#pragma once
#include "vector_streambuf.h"
#include "zmq.hpp"
#include <pagmo/s11n.hpp>
#include <boost/serialization/optional.hpp>

#include "vector_serialize.h"

namespace distributed
{
    class socket
    {
    protected:
        zmq::socket_t _socket;

        /**
         * Any socket can use this to serialize an object into byte vector.
         * @param payload Object that can be serialized
         * @return byte representation of this object
         */
        template <typename Serializable>
        static std::vector<std::byte> serialize_object(const Serializable& payload)
        {
            return vector_serialize<Serializable>(payload);
        }

    public:
        virtual ~socket() = default;

        socket(zmq::context_t& ctx, zmq::socket_type type);

        void bind(const std::string& address);

        void connect(const std::string& address);

        [[nodiscard]] zmq::socket_t& get_socket();
    };
}
