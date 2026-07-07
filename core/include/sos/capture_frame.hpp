#pragma once

#include <cstdint>

namespace sos {

/// Canonical capture frame passed from platform adapters into sos-core.
struct CaptureFrame {
  char frame_id[64];
  int64_t timestamp_ns;
  char platform[32];  // "visionos", "quest", "import"

  float pose_world_from_device[7];  // tx,ty,tz,qx,qy,qz,qw

  bool depth_available;
  float pose_confidence;
  float depth_confidence;
  float tracking_confidence;
};

struct RoomId {
  char id[64];
};

struct RoomSummary {
  char id[64];
  char display_name[128];
  char source_type[32];
  int64_t updated_at_ns;
};

enum class ImportSourceType {
  SensorLive,
  SplatFile,
  ScaniversePackage,
};

}  // namespace sos
