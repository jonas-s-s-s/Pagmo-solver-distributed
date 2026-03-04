#pragma once
#include "MsgType.h"
#include "socket.h"

namespace distributed
{
    class client_socket : public socket
    {
    public:
        client_socket(zmq::context_t& ctx, const zmq::socket_type type);;

        template <typename Serializable>
        void send(MsgType type, const Serializable& payload)
        {
            send(type, serialize_object<Serializable>(payload));
        }

        virtual void send(MsgType type);

        virtual void send(const MsgType type, const std::vector<std::byte>& serialized) = 0;

        virtual std::tuple<MsgType, std::vector<std::byte>> receive() = 0;
    };
}
