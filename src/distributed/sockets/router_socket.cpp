#include "router_socket.h"

distributed::router_socket::router_socket(zmq::context_t& ctx) : server_socket(ctx, zmq::socket_type::router)
{
}

void distributed::router_socket::send(const std::string& receiverId, const MsgType type,
                                      const std::vector<std::byte>& serialized)
{
    const int typeInt = static_cast<int>(type);
    _socket.send(zmq::buffer(receiverId), zmq::send_flags::sndmore);
    _socket.send(zmq::buffer(&typeInt, sizeof(typeInt)), zmq::send_flags::sndmore);
    _socket.send(zmq::buffer(serialized), zmq::send_flags::none);
}

std::tuple<std::string, MsgType, std::vector<std::byte>> distributed::router_socket::receive()
{
    zmq::message_t identity;
    zmq::message_t typeMsg;
    zmq::message_t payloadMsg;

    // No-discard
    const auto r0 = _socket.recv(identity);
    const auto r1 = _socket.recv(typeMsg);

    // Identification of the message
    int type;
    std::memcpy(&type, typeMsg.data(), sizeof(type));
    MsgType msgType = static_cast<MsgType>(type);

    // We have to add a special case for the ZeroMQ hello / disconnect messages.
    // These messages cannot be multipart, so they cannot follow our protocol.
    if (msgType != MsgType::WORKER_JOIN && msgType != MsgType::WORKER_LEAVE)
    {
        const auto r2 = _socket.recv(payloadMsg);

        // The message payload
        std::vector payloadBinary(
            static_cast<std::byte*>(payloadMsg.data()),
            static_cast<std::byte*>(payloadMsg.data()) + payloadMsg.size()
        );

        return {identity.to_string(), msgType, payloadBinary};
    }

    return {identity.to_string(), msgType, {}};
}
