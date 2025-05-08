#pragma once

#include <cstdint>
#include <cstddef>

namespace pulse::net::proto {

// === Wire-Level Protocol Types ===
enum class PacketType : uint8_t {
    Hello       = 0x01,
    Ack         = 0x02,
    Data        = 0x03,
    Disconnect  = 0x04,
    Ping        = 0x05,
    Pong        = 0x06
};

// === Constants ===
inline constexpr size_t MAX_PACKET_SIZE         = 1200;
inline constexpr size_t MAX_CHANNELS            = 256;
inline constexpr size_t MAX_RELIABLE_WINDOW     = 1024;
inline constexpr uint8_t PROTOCOL_VERSION       = 1;

// === Flags ===
enum HeaderFlags : uint8_t {
    FLAG_RELIABLE      = 1 << 0,
    FLAG_ACK_REQUEST   = 1 << 1,
    FLAG_HAS_TIMESTAMP = 1 << 2,
    // bits 3-7 reserved
};

// === Wire Structs ===

#pragma pack(push, 1)

struct BaseHeader {
    uint8_t type;
    uint8_t flags;
    uint8_t channelId;

    bool isReliable() const { return flags & FLAG_RELIABLE; }
    bool wantsAck() const   { return flags & FLAG_ACK_REQUEST; }
    bool hasTimestamp() const { return flags & FLAG_HAS_TIMESTAMP; }

    PacketType packetType() const {
        return static_cast<PacketType>(type);
    }
};

struct ReliableField {
    uint32_t sequence;
};

struct TimestampField {
    uint64_t clientTimeNs;
};

struct AckHeader {
    BaseHeader base;
    uint8_t ackedChannel;
    uint32_t ackSequence;
};

struct HelloHeader {
    BaseHeader base;
    uint8_t version;
};

struct DisconnectHeader {
    BaseHeader base;
};

struct PingHeader {
    BaseHeader base;
    uint64_t clientTimeNs;
};

struct PongHeader {
    BaseHeader base;
    uint64_t clientTimeNs;
    uint64_t serverTimeNs;
};

#pragma pack(pop)

} // namespace pulse::net::proto
