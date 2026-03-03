#pragma once
#include "MsgType.h"
#include "socket.h"

namespace distributed
{
    class client_socket : public socket
    {
    public:
        client_socket(zmq::context_t& ctx, const zmq::socket_type type);;

        template <typename Packable>
        void send(MsgType type, const Packable& payload)
        {
            std::vector<std::byte> serialized;
            msgpack23::pack(std::back_inserter(serialized), payload);
            send(type, serialized);
        }

        virtual void send(MsgType type);

        virtual void send(const MsgType type, const std::vector<std::byte>& serialized) = 0;

        virtual std::tuple<MsgType, std::vector<std::byte>> receive() = 0;
    };
}
