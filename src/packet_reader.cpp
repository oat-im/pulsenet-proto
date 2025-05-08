#include <pulse/net/proto/packet_reader.h>
#include <pulse/net/proto/packet.h>

namespace pulse::net::proto {

std::optional<ParsedPacket> PacketReader::parse(const uint8_t* data, size_t len) {
    if (len < sizeof(BaseHeader)) return std::nullopt;

    ParsedPacket out;
    size_t offset = 0;

    out.base = reinterpret_cast<const BaseHeader*>(data);
    offset += sizeof(BaseHeader);

    if (out.base->isReliable()) {
        if (offset + sizeof(ReliableField) > len) return std::nullopt;
        out.reliable = reinterpret_cast<const ReliableField*>(data + offset);
        offset += sizeof(ReliableField);
    }

    if (out.base->hasTimestamp()) {
        if (offset + sizeof(TimestampField) > len) return std::nullopt;
        out.timestamp = reinterpret_cast<const TimestampField*>(data + offset);
        offset += sizeof(TimestampField);
    }

    out.payload = data + offset;
    out.payloadLen = len - offset;

    return out;
}

} // namespace pulse::net::proto
