#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sos {

struct CanonicalSplat {
  float pos[3]{};
  float scale[3]{};
  float rot[4]{0.f, 0.f, 0.f, 1.f};
  uint8_t color[4]{255, 255, 255, 255};
  float opacity = 1.f;
};

struct SplatAssetHeader {
  char magic[8]{'S', 'O', 'S', 'S', 'P', 'L', 'T', '\0'};
  uint32_t version = 1;
  uint32_t splat_count = 0;
  static constexpr char kMagic[8] = {'S', 'O', 'S', 'S', 'P', 'L', 'T', '\0'};
};

std::vector<uint8_t> encode_splat_blob(const std::vector<CanonicalSplat>& splats);
bool decode_splat_blob(const std::vector<uint8_t>& blob, std::vector<CanonicalSplat>& out);

}  // namespace sos
