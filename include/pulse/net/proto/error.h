#pragma once

namespace pulse::net::proto {

enum class ErrorCode {
    Ok = 0,
    PacketTooLarge,
    InvalidChannel,
    Disconnected,
    Unknown,
};

} // namespace pulse::net::proto