#pragma once
#include "MsgType.h"
#include "zmq.hpp"
#include "socket.h"

namespace distributed
{
    class server_socket : public socket
    {
    public:
        server_socket(zmq::context_t& ctx, const zmq::socket_type type);

        template <typename Serializable>
        void send(const std::string& receiverId, const MsgType type, const Serializable& payload)
        {
            send(receiverId, type, serialize_object<Serializable>(payload));
        }

        virtual void send(const std::string& receiverId, const MsgType type,
                          const std::vector<std::byte>& serialized) = 0;

        void send(const std::string& receiverId, const MsgType type);

        virtual std::tuple<std::string, MsgType, std::vector<std::byte>> receive() = 0;
    };
}
