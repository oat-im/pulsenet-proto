#pragma once

#include <cstdint>

namespace pulse::net::proto {

class PTT {
public:
    void record(uint64_t clientSent, uint64_t serverRecv) {
        uint64_t rtt = serverRecv > clientSent ? serverRecv - clientSent : 0;
        if (smoothed_ == 0) smoothed_ = rtt;
        else smoothed_ = (smoothed_ * 7 + rtt) / 8;
    }

    uint64_t smoothedNs() const {
        return smoothed_;
    }

private:
    uint64_t smoothed_ = 0;
};

} // namespace pulse::net::proto
