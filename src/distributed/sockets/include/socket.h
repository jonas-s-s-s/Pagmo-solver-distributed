#pragma once
#include "zmq.hpp"
#include "msgpack23.h"

namespace distributed
{
    class socket
    {
    protected:
        zmq::socket_t _socket;

    public:
        virtual ~socket() = default;

        socket(zmq::context_t& ctx, zmq::socket_type type);

        void bind(const std::string& address);

        void connect(const std::string& address);

        [[nodiscard]] zmq::socket_t& get_socket();
    };
}
