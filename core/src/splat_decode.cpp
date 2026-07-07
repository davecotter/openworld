#include "sos/splat_decode.hpp"

#include <cstring>

namespace sos {

bool decode_splat_instances(const uint8_t* data, size_t size, std::vector<SplatInstance>& out) {
  if (!data || size < sizeof(SplatAssetHeader)) {
    return false;
  }
  std::vector<uint8_t> blob(data, data + size);
  std::vector<CanonicalSplat> canonical;
  if (!decode_splat_blob(blob, canonical)) {
    return false;
  }
  out.clear();
  out.reserve(canonical.size());
  for (const auto& s : canonical) {
    SplatInstance inst{};
    std::memcpy(inst.pos, s.pos, sizeof(inst.pos));
    std::memcpy(inst.scale, s.scale, sizeof(inst.scale));
    std::memcpy(inst.rot, s.rot, sizeof(inst.rot));
    inst.color[0] = static_cast<float>(s.color[0]) / 255.f;
    inst.color[1] = static_cast<float>(s.color[1]) / 255.f;
    inst.color[2] = static_cast<float>(s.color[2]) / 255.f;
    inst.color[3] = s.opacity;
    out.push_back(inst);
  }
  return !out.empty();
}

bool decode_splat_instances(const std::vector<uint8_t>& data, std::vector<SplatInstance>& out) {
  return decode_splat_instances(data.data(), data.size(), out);
}

}  // namespace sos
