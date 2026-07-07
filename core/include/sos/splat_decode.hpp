#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "sos/splat_asset.hpp"

namespace sos {

/// Shared splat decode for Quest GLES / host preview / tests.
struct SplatInstance {
  float pos[3]{};
  float scale[3]{};
  float rot[4]{0.f, 0.f, 0.f, 1.f};
  float color[4]{1.f, 1.f, 1.f, 1.f};
};

bool decode_splat_instances(const uint8_t* data, size_t size, std::vector<SplatInstance>& out);
bool decode_splat_instances(const std::vector<uint8_t>& data, std::vector<SplatInstance>& out);

}  // namespace sos
