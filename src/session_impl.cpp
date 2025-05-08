#include "session_impl.h"
#include <pulse/net/proto/packet.h>
#include <pulse/net/proto/buffer.h>
#include <cstring>

#include <iostream>

namespace pulse::net::proto {

SessionImpl::SessionImpl(udp::Addr addr) : addr_(addr) {
    lastRecvNs_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

const udp::Addr& SessionImpl::remoteAddr() const {
    return addr_;
}

std::expected<void, ErrorCode> SessionImpl::sendReliable(uint8_t channelId, BufferView payload) {
    reliable_[channelId].queue(channelId, payload);
    return {};
}

std::expected<void, ErrorCode> SessionImpl::sendUnreliable(uint8_t channelId, BufferView payload) {
    unreliable_[channelId].queue(channelId, payload);
    return {};
}

std::expected<void, ErrorCode> SessionImpl::sendImmediate(BufferView payload) {
    // Not used yet
    return std::unexpected(ErrorCode::Unknown);
}

uint64_t SessionImpl::idleTimeNs() const {
    uint64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    return now - lastRecvNs_;
}

uint64_t SessionImpl::latencyNs() const {
    return ptt_.smoothedNs();
}

void SessionImpl::disconnect() {
    dead_ = true;
}

bool SessionImpl::isDisconnected() const {
    return dead_;
}

std::optional<Session::ChannelStats> SessionImpl::getChannelStats(uint8_t channelId) const {
    auto it = reliable_.find(channelId);
    if (it == reliable_.end()) return std::nullopt;
    auto s = it->second.stats();
    return ChannelStats{s.lastSent, s.lastAcked, s.unacked};
}

void SessionImpl::onReceive(const ParsedPacket& pkt, udp::Socket& socket, OnPayloadFn& onPayload, uint64_t nowNs) {
    lastRecvNs_ = nowNs;

    if (pkt.base->type == static_cast<uint8_t>(PacketType::Ping)) {
        PongHeader pong;
        pong.base = *pkt.base;
        pong.base.type = static_cast<uint8_t>(PacketType::Pong);
        pong.clientTimeNs = *reinterpret_cast<const uint64_t*>(pkt.payload);
        pong.serverTimeNs = nowNs;
        socket.sendTo(addr_, reinterpret_cast<const uint8_t*>(&pong), sizeof(pong));
        return;
    }

    if (pkt.base->type == static_cast<uint8_t>(PacketType::Pong)) {
        auto ph = reinterpret_cast<const PongHeader*>(pkt.base);
        ptt_.record(ph->clientTimeNs, ph->serverTimeNs);
        return;
    }

    if (pkt.base->type == static_cast<uint8_t>(PacketType::Ack)) {
        if (pkt.payloadLen < sizeof(AckHeader)) return;
    
        const auto* ack = reinterpret_cast<const AckHeader*>(pkt.base);
        uint8_t channelId = ack->ackedChannel;
        uint32_t ackSeq = ack->ackSequence;
    
        auto it = reliable_.find(channelId);
        if (it != reliable_.end()) {
            it->second.acknowledge(ackSeq);
        }
        return;
    }

    if (pkt.reliable) {
        auto& window = recvWindows_[pkt.base->channelId];
        if (!window.shouldAccept(pkt.reliable->sequence)) return;

        // Send ACK
        AckHeader ack{
            *pkt.base,
            pkt.base->channelId,
            pkt.reliable->sequence
        };
        ack.base.type = static_cast<uint8_t>(PacketType::Ack);
        ack.base.flags = 0; // No flags for ACK
        socket.sendTo(addr_, reinterpret_cast<uint8_t*>(&ack), sizeof(ack));
    }

    std::cout << "[Debug] Received packet on channel " << static_cast<int>(pkt.base->channelId) << ". Packet Type = " << static_cast<int>(pkt.base->type) << "\n";
    onPayload(*this, pkt.base->channelId, BufferView(pkt.payload, pkt.payloadLen));
}

void SessionImpl::flush(udp::Socket& socket, uint64_t nowNs) {
    for (auto& [ch, c] : unreliable_) {
        c.flush(socket, addr_);
    }
    for (auto& [ch, c] : reliable_) {
        c.flush(socket, addr_, nowNs);
    }
}

} // namespace pulse::net::proto
