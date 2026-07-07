#include "sos/hash.hpp"

#include <iomanip>
#include <sstream>

namespace sos {

namespace {

constexpr uint64_t kPrime1 = 11400714785074694791ULL;
constexpr uint64_t kPrime2 = 14029467366897019727ULL;
constexpr uint64_t kPrime3 = 1609587929392839161ULL;
constexpr uint64_t kPrime4 = 9650029242287828579ULL;
constexpr uint64_t kPrime5 = 2870177450012600261ULL;

uint64_t rotl64(uint64_t x, int r) {
  return (x << r) | (x >> (64 - r));
}

uint64_t round(uint64_t acc, uint64_t input) {
  acc += input * kPrime2;
  acc = rotl64(acc, 31);
  return acc * kPrime1;
}

uint64_t merge(uint64_t acc, uint64_t val) {
  acc ^= round(0, val);
  acc = acc * kPrime1 + kPrime4;
  return acc;
}

}  // namespace

uint64_t xxhash64(const void* data, size_t len, uint64_t seed) {
  const auto* p = static_cast<const uint8_t*>(data);
  const auto* const end = p + len;
  uint64_t h = seed + kPrime5 + static_cast<uint64_t>(len);

  if (len >= 32) {
    const auto* limit = end - 32;
    uint64_t v1 = seed + kPrime1 + kPrime2;
    uint64_t v2 = seed + kPrime2;
    uint64_t v3 = seed;
    uint64_t v4 = seed - kPrime1;

    do {
      v1 = round(v1, *reinterpret_cast<const uint64_t*>(p));
      p += 8;
      v2 = round(v2, *reinterpret_cast<const uint64_t*>(p));
      p += 8;
      v3 = round(v3, *reinterpret_cast<const uint64_t*>(p));
      p += 8;
      v4 = round(v4, *reinterpret_cast<const uint64_t*>(p));
      p += 8;
    } while (p <= limit);

    h = rotl64(v1, 1) + rotl64(v2, 7) + rotl64(v3, 12) + rotl64(v4, 18);
    h = merge(h, v1);
    h = merge(h, v2);
    h = merge(h, v3);
    h = merge(h, v4);
  }

  h += static_cast<uint64_t>(end - p);

  while (p + 8 <= end) {
    h = merge(h, *reinterpret_cast<const uint64_t*>(p));
    p += 8;
  }

  if (p + 4 <= end) {
    h ^= static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(p)) * kPrime1;
    h = rotl64(h, 23) * kPrime2 + kPrime3;
    p += 4;
  }

  while (p < end) {
    h ^= static_cast<uint64_t>(*p) * kPrime5;
    h = rotl64(h, 11) * kPrime1;
    ++p;
  }

  h ^= h >> 33;
  h *= kPrime2;
  h ^= h >> 29;
  h *= kPrime3;
  h ^= h >> 32;
  return h;
}

std::string hash_to_hex(uint64_t hash) {
  std::ostringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(16) << hash;
  return ss.str();
}

}  // namespace sos
