#pragma once

#include "buffer.h"
#include "session.h"
#include <pulse/net/udp/udp_addr.h>
#include <functional>

namespace pulse::net::proto {

using OnPayloadFn    = std::function<void(Session&, uint8_t, BufferView)>;
using OnDisconnectFn = std::function<void(const udp::Addr&)>;

} // namespace pulse::net::proto
