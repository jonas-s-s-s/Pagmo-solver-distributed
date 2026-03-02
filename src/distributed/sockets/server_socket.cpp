#include "server_socket.h"

distributed::server_socket::server_socket(zmq::context_t& ctx, const zmq::socket_type type): socket(ctx, type)
{}

void distributed::server_socket::send(const std::string& receiverId, const MsgType type)
{
    send(receiverId, type, std::vector<std::byte>());
}
