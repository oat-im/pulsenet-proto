#include "channel_reliable.h"
#include <cstring>

namespace pulse::net::proto {

ReliableChannel::ReliableChannel()
    : nextSeq_(1), lastAck_(0) {}

std::expected<void, ErrorCode> ReliableChannel::queue(uint8_t channelId, BufferView payload) {
    if (payload.size > MAX_PACKET_SIZE) return std::unexpected(ErrorCode::PacketTooLarge);

    std::vector<uint8_t> buf;
    buf.reserve(sizeof(BaseHeader) + sizeof(ReliableField) + payload.size);

    BaseHeader base{static_cast<uint8_t>(PacketType::Data), FLAG_RELIABLE, channelId};
    ReliableField rel{nextSeq_};

    buf.insert(buf.end(), reinterpret_cast<uint8_t*>(&base), reinterpret_cast<uint8_t*>(&base) + sizeof(BaseHeader));
    buf.insert(buf.end(), reinterpret_cast<uint8_t*>(&rel), reinterpret_cast<uint8_t*>(&rel) + sizeof(ReliableField));
    buf.insert(buf.end(), payload.data, payload.data + payload.size);

    queue_.push_back({std::move(buf), nextSeq_, 0, 0});
    nextSeq_++;
    return {};
}   

void ReliableChannel::flush(udp::ISocket& socket, const udp::Addr& to, uint64_t nowNs) {
    for (auto& pkt : queue_) {
        if (pkt.lastSentNs == 0 || nowNs - pkt.lastSentNs >= RESEND_TIMEOUT_NS) {
            socket.sendTo(to, pkt.data.data(), pkt.data.size());
            pkt.lastSentNs = nowNs;
            pkt.retryCount++;
        }
    }
}

void ReliableChannel::acknowledge(uint32_t ackSeq) {
    while (!queue_.empty() && queue_.front().sequence <= ackSeq) {
        queue_.pop_front(); // clear sent and now-acked packets
        lastAck_ = ackSeq;
    }
}


ReliableChannel::Stats ReliableChannel::stats() const {
    return {
        nextSeq_ - 1,
        lastAck_,
        static_cast<uint32_t>(queue_.size())
    };
}

} // namespace pulse::net::proto
