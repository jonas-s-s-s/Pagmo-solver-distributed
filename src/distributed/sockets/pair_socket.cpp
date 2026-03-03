#include "pair_socket.h"

distributed::pair_socket::pair_socket(zmq::context_t& ctx) : client_socket(ctx, zmq::socket_type::pair)
{
}

void distributed::pair_socket::send(const MsgType type, const std::vector<std::byte>& serialized)
{
    const int typeInt = static_cast<int>(type);
    _socket.send(zmq::buffer(&typeInt, sizeof(typeInt)), zmq::send_flags::sndmore);
    _socket.send(zmq::buffer(serialized), zmq::send_flags::none);
}

std::tuple<MsgType, std::vector<std::byte>> distributed::pair_socket::receive()
{
    zmq::message_t typeMsg;
    zmq::message_t payloadMsg;

    // No-discard
    const auto r1 = _socket.recv(typeMsg);
    const auto r2 = _socket.recv(payloadMsg);

    // Identification of the message
    int type;
    std::memcpy(&type, typeMsg.data(), sizeof(type));

    // The serialized message object
    std::vector payloadBinary(
        static_cast<std::byte*>(payloadMsg.data()),
        static_cast<std::byte*>(payloadMsg.data()) + payloadMsg.size()
    );

    return {static_cast<MsgType>(type), payloadBinary};
}
