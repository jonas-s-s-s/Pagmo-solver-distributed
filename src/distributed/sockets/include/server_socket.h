#pragma once
#include "MsgType.h"
#include "zmq.hpp"
#include "msgpack23.h"
#include "socket.h"

namespace distributed
{
    class server_socket : public socket
    {
    public:
        server_socket(zmq::context_t& ctx, const zmq::socket_type type);

        template <typename Packable>
        void send(const std::string& receiverId, const MsgType type, const Packable& payload)
        {
            std::vector<std::byte> serialized;
            msgpack23::pack(std::back_inserter(serialized), payload);
            send(receiverId, type, serialized);
        }

        virtual void send(const std::string& receiverId, const MsgType type, const std::vector<std::byte>& serialized) = 0;

        void send(const std::string& receiverId, const MsgType type);

        virtual std::tuple<std::string, MsgType, std::vector<std::byte>> receive() = 0;
    };

}

