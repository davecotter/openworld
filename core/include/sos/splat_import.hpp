#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace sos {

/// Parse PLY or .splat into canonical SOS splat blob (see spec-12 §3.5).
struct SplatImportResult {
  bool ok = false;
  std::string error;
  std::vector<uint8_t> canonical_blob;
  std::string content_hash;
  uint32_t splat_count = 0;
};

SplatImportResult import_splat_file(const std::string& path);

}  // namespace sos
