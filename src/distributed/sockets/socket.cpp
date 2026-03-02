#include "socket.h"

distributed::socket::socket(zmq::context_t& ctx, zmq::socket_type type) : _socket(ctx, type)
{
}

void distributed::socket::bind(const std::string& address)
{
    _socket.bind(address);
}

zmq::socket_t& distributed::socket::get_socket()
{
    return _socket;
}

