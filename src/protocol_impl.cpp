#include "protocol_impl.h"
#include <pulse/net/proto/packet.h>
#include <pulse/net/proto/buffer.h>
#include <pulse/net/proto/session.h>
#include <cstring>

namespace pulse::net::proto {

ProtocolImpl::ProtocolImpl(udp::ISocket& socket, OnPayloadFn onPayload, OnDisconnectFn onDisconnect, const Config& config)
    : socket_(socket), onPayload_(onPayload), onDisconnect_(onDisconnect), config_(config) {}

void ProtocolImpl::tick() {
    uint64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    while (true) {
        auto maybe = socket_.recvFrom();
        if (!maybe.has_value()) break;

        auto [data, len, unused, from] = *maybe;
        (void)unused; // unused in this context
        if (len < sizeof(BaseHeader)) continue;

        auto parsed = PacketReader::parse(data, len);
        if (!parsed) continue;

        auto& pkt = *parsed;
        auto& sess = sessions_[from];
        if (!sess) sess = std::make_unique<SessionImpl>(from);

        auto impl = static_cast<SessionImpl*>(sess.get());
        if (impl->isDisconnected()) continue;

        impl->onReceive(pkt, socket_, onPayload_, now);
    }

    for (auto it = sessions_.begin(); it != sessions_.end(); ) {
        auto* impl = static_cast<SessionImpl*>(it->second.get());
        if (impl->idleTimeNs() > config_.disconnectTimeoutNs) {
            onDisconnect_(it->first);
            it = sessions_.erase(it);
        } else {
            impl->flush(socket_, now);
            ++it;
        }
    }
}

std::unordered_map<udp::Addr, std::unique_ptr<Session>>& ProtocolImpl::sessions() {
    return sessions_;
}

std::expected<Session*, ErrorCode> ProtocolImpl::connect(const udp::Addr& addr) {
    auto it = sessions_.find(addr);
    if (it != sessions_.end()) {
        return it->second.get(); // already connected
    }

    auto sess = std::make_unique<SessionImpl>(addr);
    Session* ptr = sess.get();
    sessions_[addr] = std::move(sess);
    return ptr;
}


std::expected<std::unique_ptr<Protocol>, ErrorCode> CreateProtocol(
    udp::ISocket& socket,
    OnPayloadFn onPayload,
    OnDisconnectFn onDisconnect,
    const Protocol::Config& config
) {
    return std::make_unique<ProtocolImpl>(socket, onPayload, onDisconnect, config);
}

} // namespace pulse::net::proto
