#include "sos/splat_asset.hpp"

#include <cstring>

namespace sos {

std::vector<uint8_t> encode_splat_blob(const std::vector<CanonicalSplat>& splats) {
  std::vector<uint8_t> blob;
  blob.resize(sizeof(SplatAssetHeader) + splats.size() * sizeof(CanonicalSplat));
  SplatAssetHeader header{};
  header.version = 1;
  header.splat_count = static_cast<uint32_t>(splats.size());
  std::memcpy(blob.data(), &header, sizeof(header));
  if (!splats.empty()) {
    std::memcpy(blob.data() + sizeof(header), splats.data(),
                splats.size() * sizeof(CanonicalSplat));
  }
  return blob;
}

bool decode_splat_blob(const std::vector<uint8_t>& blob, std::vector<CanonicalSplat>& out) {
  if (blob.size() < sizeof(SplatAssetHeader)) {
    return false;
  }
  SplatAssetHeader header{};
  std::memcpy(&header, blob.data(), sizeof(header));
  if (std::memcmp(header.magic, SplatAssetHeader::kMagic, sizeof(header.magic)) != 0) {
    return false;
  }
  const size_t expected = sizeof(SplatAssetHeader) + header.splat_count * sizeof(CanonicalSplat);
  if (blob.size() < expected) {
    return false;
  }
  out.resize(header.splat_count);
  if (header.splat_count > 0) {
    std::memcpy(out.data(), blob.data() + sizeof(header),
                header.splat_count * sizeof(CanonicalSplat));
  }
  return true;
}

}  // namespace sos
