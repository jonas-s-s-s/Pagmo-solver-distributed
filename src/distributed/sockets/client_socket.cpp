#include "client_socket.h"

distributed::client_socket::client_socket(zmq::context_t& ctx, const zmq::socket_type type): socket(ctx, type)
{}

void distributed::client_socket::send(MsgType type)
{
    send(type, std::vector<std::byte>());
}
