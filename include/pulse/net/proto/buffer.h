#pragma once

#include <cstdint>
#include <cstddef>

namespace pulse::net::proto {

struct BufferView {
    const uint8_t* data = nullptr;
    size_t size = 0;

    BufferView() = default;
    BufferView(const uint8_t* d, size_t s) : data(d), size(s) {}
};

} // namespace pulse::net::proto