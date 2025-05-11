#pragma once

#include <pulse/net/proto/buffer.h>
#include <pulse/net/proto/error.h>
#include <pulse/net/udp/udp.h>
#include <vector>
#include <deque>

namespace pulse::net::proto {

class UnreliableChannel {
public:
    void queue(uint8_t channelId, BufferView payload);
    void flush(udp::ISocket& socket, const udp::Addr& to);

private:
    std::deque<std::vector<uint8_t>> buffer_;
};

} // namespace pulse::net::proto
