#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace sos {

uint64_t xxhash64(const void* data, size_t len, uint64_t seed = 0);
std::string hash_to_hex(uint64_t hash);

}  // namespace sos
