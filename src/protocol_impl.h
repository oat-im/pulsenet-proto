#pragma once

#include <pulse/net/proto/proto.h>
#include "session_impl.h"

namespace pulse::net::proto {

class ProtocolImpl : public Protocol {
public:
    ProtocolImpl(udp::ISocket& socket, OnPayloadFn onPayload, OnDisconnectFn onDisconnect, const Config& config);

    void tick() override;
    std::unordered_map<udp::Addr, std::unique_ptr<Session>>& sessions() override;
    std::expected<Session*, ErrorCode> connect(const udp::Addr& addr) override;

private:
    udp::ISocket& socket_;
    OnPayloadFn onPayload_;
    OnDisconnectFn onDisconnect_;
    Config config_;

    std::unordered_map<udp::Addr, std::unique_ptr<Session>> sessions_;
};

} // namespace pulse::net::proto
