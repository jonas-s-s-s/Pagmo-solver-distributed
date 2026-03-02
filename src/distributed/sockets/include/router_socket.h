#pragma once
#include "server_socket.h"

namespace distributed
{
    class router_socket final : public server_socket
    {

    public:
        explicit router_socket(zmq::context_t& ctx);

        void send(const std::string& receiverId, MsgType type, const std::vector<std::byte>& serialized) override;
        std::tuple<std::string, MsgType, std::vector<std::byte>> receive() override;
    };
}
