#include "channel_unreliable.h"
#include <pulse/net/proto/packet.h>

namespace pulse::net::proto {

void UnreliableChannel::queue(uint8_t channelId, BufferView payload) {
    if (payload.size > MAX_PACKET_SIZE) return;

    std::vector<uint8_t> buf;
    buf.reserve(sizeof(BaseHeader) + payload.size);

    BaseHeader base{static_cast<uint8_t>(PacketType::Data), 0, channelId};

    buf.insert(buf.end(), reinterpret_cast<uint8_t*>(&base), reinterpret_cast<uint8_t*>(&base) + sizeof(BaseHeader));
    buf.insert(buf.end(), payload.data, payload.data + payload.size);

    buffer_.push_back(std::move(buf));
}
    

void UnreliableChannel::flush(udp::ISocket& socket, const udp::Addr& to) {
    while (!buffer_.empty()) {
        auto& pkt = buffer_.front();
        socket.sendTo(to, pkt.data(), pkt.size());
        buffer_.pop_front();
    }
}

} // namespace pulse::net::proto
