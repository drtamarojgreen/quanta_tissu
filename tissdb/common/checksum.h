#pragma once

#include <cstdint>
#include <cstddef>

namespace TissDB {
namespace Common {

namespace Crc32Impl {
    uint32_t calculate(const void* data, size_t size, uint32_t crc = 0xFFFFFFFF);
}

inline uint32_t crc32(const void* data, size_t size) {
    return Crc32Impl::calculate(data, size);
}

}
}
