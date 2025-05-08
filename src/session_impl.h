#pragma once

#include <pulse/net/proto/session.h>
#include <pulse/net/proto/packet.h>
#include <pulse/net/proto/packet_reader.h>
#include <pulse/net/proto/callbacks.h>
#include "ptt.h"
#include "channel_reliable.h"
#include "channel_unreliable.h"
#include "recv_window.h"
#include <unordered_map>
#include <deque>
#include <chrono>

namespace pulse::net::proto {

class SessionImpl : public Session {
public:
    SessionImpl(udp::Addr addr);

    const udp::Addr& remoteAddr() const override;

    std::expected<void, ErrorCode> sendReliable(uint8_t channelId, BufferView payload) override;
    std::expected<void, ErrorCode> sendUnreliable(uint8_t channelId, BufferView payload) override;
    std::expected<void, ErrorCode> sendImmediate(BufferView payload) override;

    uint64_t idleTimeNs() const override;
    uint64_t latencyNs() const override;

    void disconnect() override;
    bool isDisconnected() const;

    std::optional<ChannelStats> getChannelStats(uint8_t channelId) const override;

    void onReceive(const ParsedPacket& pkt, udp::Socket& socket, OnPayloadFn& onPayload, uint64_t nowNs);
    void flush(udp::Socket& socket, uint64_t nowNs);

private:
    udp::Addr addr_;
    bool dead_ = false;

    uint64_t lastRecvNs_ = 0;

    std::unordered_map<uint8_t, ReliableChannel> reliable_;
    std::unordered_map<uint8_t, UnreliableChannel> unreliable_;
    std::unordered_map<uint8_t, RecvWindow> recvWindows_;

    PTT ptt_;
};

} // namespace pulse::net::proto
