#pragma once

#include <string>
#include <vector>

#include "sos/capture_frame.hpp"
#include "sos/room_record.hpp"

namespace sos {

struct ReconstructionDelta {
  RoomId room_id;
  std::string geometry_asset_hash;
  std::string asset_kind;  // splat_chunk | mesh_proxy
  float geometry_confidence = 0.f;
  ImportSourceType source = ImportSourceType::SensorLive;
  RoomRecord record;
  std::vector<std::pair<std::string, std::vector<uint8_t>>> assets;
};

class NormalizationPipeline {
 public:
  void ingest_frame(const CaptureFrame& frame);
  ReconstructionDelta ingest_splat_file(const std::string& path);
  ReconstructionDelta finalize_room(const std::string& display_name);

 private:
  std::vector<CaptureFrame> frames_;
  ImportSourceType source_ = ImportSourceType::SensorLive;
};

}  // namespace sos
