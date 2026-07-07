#include "sos/normalization_pipeline.hpp"

#include <chrono>
#include <sstream>

#include "sos/mesh_proxy.hpp"
#include "sos/splat_import.hpp"

namespace sos {

namespace {

int64_t NowNs() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

}  // namespace

void NormalizationPipeline::ingest_frame(const CaptureFrame& frame) {
  source_ = ImportSourceType::SensorLive;
  frames_.push_back(frame);
}

ReconstructionDelta NormalizationPipeline::ingest_splat_file(const std::string& path) {
  source_ = ImportSourceType::SplatFile;
  frames_.clear();

  const auto imported = import_splat_file(path);
  ReconstructionDelta delta;
  if (!imported.ok) {
    return delta;
  }

  RoomId id{};
  std::ostringstream ss;
  ss << "room-splat-" << imported.content_hash.substr(0, 12);
  const auto id_str = ss.str();
  id_str.copy(id.id, sizeof(id.id) - 1);

  delta.room_id = id;
  delta.geometry_asset_hash = imported.content_hash;
  delta.asset_kind = "splat_chunk";
  delta.geometry_confidence = 0.85f;
  delta.source = ImportSourceType::SplatFile;
  delta.assets.emplace_back(imported.content_hash, imported.canonical_blob);

  delta.record.id = id_str;
  delta.record.display_name = "Imported Splat";
  delta.record.source_type = "splat_file";
  delta.record.primary_asset_hash = imported.content_hash;
  delta.record.asset_kind = "splat_chunk";
  delta.record.created_at_ns = NowNs();
  delta.record.updated_at_ns = delta.record.created_at_ns;
  delta.record.geometry_confidence = delta.geometry_confidence;
  return delta;
}

ReconstructionDelta NormalizationPipeline::finalize_room(const std::string& display_name) {
  ReconstructionDelta delta;
  if (frames_.empty()) {
    return delta;
  }

  const auto mesh = generate_mesh_proxy(frames_);
  if (mesh.content_hash.empty()) {
    return delta;
  }

  RoomId id{};
  const auto now = NowNs();
  std::ostringstream ss;
  ss << "room-scan-" << now;
  const auto id_str = ss.str();
  id_str.copy(id.id, sizeof(id.id) - 1);

  delta.room_id = id;
  delta.geometry_asset_hash = mesh.content_hash;
  delta.asset_kind = "mesh_proxy";
  delta.geometry_confidence = 0.7f;
  delta.source = source_;
  delta.assets.emplace_back(mesh.content_hash, mesh.blob);

  delta.record.id = id_str;
  delta.record.display_name = display_name.empty() ? "Scanned Room" : display_name;
  delta.record.source_type = "sensor_live";
  delta.record.primary_asset_hash = mesh.content_hash;
  delta.record.asset_kind = "mesh_proxy";
  delta.record.created_at_ns = now;
  delta.record.updated_at_ns = now;
  delta.record.geometry_confidence = delta.geometry_confidence;
  if (!frames_.empty()) {
    std::memcpy(delta.record.pose, frames_.back().pose_world_from_device,
                sizeof(delta.record.pose));
  }
  return delta;
}

}  // namespace sos
