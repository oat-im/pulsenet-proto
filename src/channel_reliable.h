#pragma once

#include <pulse/net/proto/buffer.h>
#include <pulse/net/proto/error.h>
#include <pulse/net/proto/packet.h>
#include <pulse/net/udp/udp.h>
#include <chrono>
#include <deque>
#include <vector>

namespace pulse::net::proto {

class ReliableChannel {
public:
    ReliableChannel();

    std::expected<void, ErrorCode> queue(uint8_t channelId, BufferView payload);
    void flush(udp::Socket& socket, const udp::Addr& to, uint64_t nowNs);
    void acknowledge(uint32_t ackSeq);

    struct Stats {
        uint32_t lastSent;
        uint32_t lastAcked;
        uint32_t unacked;
    };
    Stats stats() const;

private:
    struct PacketEntry {
        std::vector<uint8_t> data;
        uint32_t sequence;
        uint64_t lastSentNs;
        int retryCount;
    };

    std::deque<PacketEntry> queue_;
    uint32_t nextSeq_;
    uint32_t lastAck_;
    static constexpr uint64_t RESEND_TIMEOUT_NS = 50'000'000; // 50ms
};

} // namespace pulse::net::proto
