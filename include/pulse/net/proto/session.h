#pragma once

#include "buffer.h"
#include "error.h"
#include <pulse/net/udp/udp_addr.h>
#include <expected>
#include <cstdint>
#include <optional>

namespace pulse::net::proto {

class Session {
public:
    virtual ~Session() = default;

    virtual const pulse::net::udp::Addr& remoteAddr() const = 0;

    virtual std::expected<void, ErrorCode> sendReliable(uint8_t channelId, BufferView payload) = 0;
    virtual std::expected<void, ErrorCode> sendUnreliable(uint8_t channelId, BufferView payload) = 0;
    virtual std::expected<void, ErrorCode> sendImmediate(BufferView payload) = 0;

    virtual uint64_t idleTimeNs() const = 0;
    virtual uint64_t latencyNs() const = 0;

    virtual void disconnect() = 0;

    struct ChannelStats {
        uint32_t lastSent;
        uint32_t lastAcked;
        uint32_t queuedUnacked;
    };

    virtual std::optional<ChannelStats> getChannelStats(uint8_t channelId) const = 0;
};

} // namespace pulse::net::proto