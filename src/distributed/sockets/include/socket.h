#pragma once
#include "vector_streambuf.h"
#include "zmq.hpp"
#include <pagmo/s11n.hpp>

namespace distributed
{
    class socket
    {
    protected:
        zmq::socket_t _socket;

        /**
         * Any socket can use this to serialize an object into byte array.
         * Defined only here, any changes will effect serialization of all sockets.
         * @param payload Object that can be serialized
         * @return byte representation of this object
         */
        template <typename Serializable>
        static std::vector<std::byte> serialize_object(const Serializable& payload)
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

    public:
        virtual ~socket() = default;

        socket(zmq::context_t& ctx, zmq::socket_type type);

        void bind(const std::string& address);

        void connect(const std::string& address);

        [[nodiscard]] zmq::socket_t& get_socket();

    };
}
