#pragma once

#include <cstdint>
#include <string>

#include "sos/capture_frame.hpp"

namespace sos {

struct RoomRecord {
  static constexpr int kSwmVersion = 1;

  std::string id;
  std::string display_name;
  std::string source_type;  // sensor_live | splat_file | scaniverse | bundle_import
  std::string primary_asset_hash;
  std::string asset_kind;  // splat_chunk | mesh_proxy
  int64_t created_at_ns = 0;
  int64_t updated_at_ns = 0;
  float geometry_confidence = 0.f;
  float pose[7]{};  // tx,ty,tz,qx,qy,qz,qw
  double gps_latitude = 0.0;
  double gps_longitude = 0.0;
  float gps_accuracy_m = 0.f;
  bool has_gps = false;
};

std::string room_record_to_json(const RoomRecord& record);
bool room_record_from_json(const std::string& json, RoomRecord& out);

}  // namespace sos
