#include "sos/mesh_proxy.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "sos/hash.hpp"

namespace sos {

namespace {

struct MeshHeader {
  char magic[8] = {'S', 'O', 'S', 'M', 'E', 'S', 'H', '\0'};
  uint32_t version = 1;
  uint32_t point_count = 0;
};

struct MeshPoint {
  float pos[3]{};
  float normal[3]{0.f, 1.f, 0.f};
};

}  // namespace

MeshProxyAsset generate_mesh_proxy(const std::vector<CaptureFrame>& frames) {
  MeshProxyAsset asset;
  if (frames.empty()) {
    return asset;
  }

  std::vector<MeshPoint> points;
  points.reserve(frames.size());
  for (const auto& frame : frames) {
    MeshPoint point{};
    point.pos[0] = frame.pose_world_from_device[0];
    point.pos[1] = frame.pose_world_from_device[1];
    point.pos[2] = frame.pose_world_from_device[2];
    points.push_back(point);
  }

  if (points.size() >= 3) {
    float minv[3] = {points[0].pos[0], points[0].pos[1], points[0].pos[2]};
    float maxv[3] = {minv[0], minv[1], minv[2]};
    for (const auto& p : points) {
      for (int i = 0; i < 3; ++i) {
        minv[i] = std::min(minv[i], p.pos[i]);
        maxv[i] = std::max(maxv[i], p.pos[i]);
      }
    }
    const float cx = (minv[0] + maxv[0]) * 0.5f;
    const float cy = (minv[1] + maxv[1]) * 0.5f;
    const float cz = (minv[2] + maxv[2]) * 0.5f;
    const float hx = std::max(0.5f, (maxv[0] - minv[0]) * 0.5f);
    const float hy = std::max(0.5f, (maxv[1] - minv[1]) * 0.5f);
    const float hz = std::max(0.5f, (maxv[2] - minv[2]) * 0.5f);

    const float corners[8][3] = {
        {cx - hx, cy - hy, cz - hz}, {cx + hx, cy - hy, cz - hz},
        {cx - hx, cy + hy, cz - hz}, {cx + hx, cy + hy, cz - hz},
        {cx - hx, cy - hy, cz + hz}, {cx + hx, cy - hy, cz + hz},
        {cx - hx, cy + hy, cz + hz}, {cx + hx, cy + hy, cz + hz},
    };
    points.clear();
    for (const auto& c : corners) {
      MeshPoint point{};
      point.pos[0] = c[0];
      point.pos[1] = c[1];
      point.pos[2] = c[2];
      points.push_back(point);
    }
  }

  MeshHeader header{};
  header.point_count = static_cast<uint32_t>(points.size());
  asset.blob.resize(sizeof(header) + points.size() * sizeof(MeshPoint));
  std::memcpy(asset.blob.data(), &header, sizeof(header));
  std::memcpy(asset.blob.data() + sizeof(header), points.data(),
              points.size() * sizeof(MeshPoint));
  asset.point_count = header.point_count;
  asset.content_hash =
      hash_to_hex(xxhash64(asset.blob.data(), asset.blob.size()));
  return asset;
}

}  // namespace sos
