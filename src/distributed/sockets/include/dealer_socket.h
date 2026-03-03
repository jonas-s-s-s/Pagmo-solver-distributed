#pragma once
#include "client_socket.h"

namespace distributed
{
    class dealer_socket final : public client_socket
    {

    public:
        using client_socket::send;

        explicit dealer_socket(zmq::context_t& ctx);

        void send(const MsgType type, const std::vector<std::byte>& serialized) override;
        std::tuple<MsgType, std::vector<std::byte>> receive() override;

        void set_routing_id(const std::string& routingId);
    };
}
