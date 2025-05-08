#pragma once

#include <cstdint>
#include <bitset>

namespace pulse::net::proto {

class RecvWindow {
public:
    RecvWindow() : lastSeq_(0), initialized_(false) {}

    bool shouldAccept(uint32_t seq) {
        if (!initialized_) {
            lastSeq_ = seq;
            initialized_ = true;
            return true;
        }

        int32_t diff = static_cast<int32_t>(seq - lastSeq_);
        if (diff > 0) {
            shiftWindow(diff);
            lastSeq_ = seq;
            return true;
        } else if (diff == 0) {
            return false;
        } else if (diff >= -31) {
            size_t index = static_cast<size_t>(lastSeq_ - seq);
            if (received_.test(index)) return false;
            received_.set(index);
            return true;
        }

        return false;
    }

private:
    uint32_t lastSeq_;
    bool initialized_;
    std::bitset<32> received_;

    void shiftWindow(int32_t amount) {
        received_ <<= amount;
        received_.set(0);
    }
};

} // namespace pulse::net::proto
