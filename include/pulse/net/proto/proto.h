#pragma once

#include <pulse/net/proto/session.h>
#include <pulse/net/proto/callbacks.h>
#include <pulse/net/proto/logger.h>
#include <pulse/net/proto/metrics.h>
#include <pulse/net/proto/buffer.h>
#include <pulse/net/udp/udp.h>
#include <functional>
#include <memory>

namespace pulse::net::proto {

// === Top-level Protocol Driver Interface ===
// Use this to spin up a session manager

class Protocol {
public:
    struct Config {
        uint64_t disconnectTimeoutNs = 10'000'000'000ull; // 10 sec
        Logger* logger = nullptr;
        Metrics* metrics = nullptr;
    };

    virtual ~Protocol() = default;

    // Tick the protocol loop (recv, dispatch, flush)
    virtual void tick() = 0;

    // Access sessions
    virtual std::unordered_map<udp::Addr, std::unique_ptr<Session>>& sessions() = 0;

    // Connect to a remote address
    virtual std::expected<Session*, ErrorCode> connect(const udp::Addr& addr) = 0;
};

std::expected<std::unique_ptr<Protocol>, ErrorCode> CreateProtocol(
    udp::Socket& socket,
    OnPayloadFn onPayload,
    OnDisconnectFn onDisconnect,
    const Protocol::Config& config = {}
);

} // namespace pulse::net::proto
