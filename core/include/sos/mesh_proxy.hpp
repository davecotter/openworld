#pragma once

#include <string>
#include <vector>

#include "sos/capture_frame.hpp"

namespace sos {

struct MeshProxyAsset {
  std::vector<uint8_t> blob;
  std::string content_hash;
  uint32_t point_count = 0;
};

MeshProxyAsset generate_mesh_proxy(const std::vector<CaptureFrame>& frames);

}  // namespace sos
