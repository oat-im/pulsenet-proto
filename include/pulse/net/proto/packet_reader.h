#pragma once

#include <cstdint>
#include <cstddef>
#include <optional>

namespace pulse::net::proto {

struct ParsedPacket {
    const struct BaseHeader* base = nullptr;
    const struct ReliableField* reliable = nullptr;
    const struct TimestampField* timestamp = nullptr;
    const uint8_t* payload = nullptr;
    size_t payloadLen = 0;
};

class PacketReader {
public:
    static std::optional<ParsedPacket> parse(const uint8_t* data, size_t len);
};

} // namespace pulse::net::proto
