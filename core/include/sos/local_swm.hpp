#pragma once

#include <string>
#include <vector>

#include "sos/capture_frame.hpp"

namespace sos {

/// Content-addressed local Spatial World Model store (Phase 0: single device).
class LocalSwmStore {
 public:
  explicit LocalSwmStore(std::string root_path);

  bool write_room(const RoomId& id,
                  const std::string& room_json,
                  const std::vector<std::pair<std::string, std::vector<uint8_t>>>& assets);

  bool read_room_json(const RoomId& id, std::string& out_json) const;
  bool read_asset(const std::string& content_hash, std::vector<uint8_t>& out_bytes) const;
  bool read_events(const RoomId& id, std::string& out_events) const;
  bool write_events(const RoomId& id, const std::string& events);
  bool append_event(const RoomId& id, const std::string& line);

  std::vector<RoomSummary> list_rooms() const;

  bool export_bundle(const RoomId& id, const std::string& bundle_path) const;
  bool import_bundle(const std::string& bundle_path, RoomId& out_id);

  const std::string& root_path() const { return root_path_; }

 private:
  void rebuild_index() const;

  std::string root_path_;
};

bool export_room_bundle(const LocalSwmStore& store,
                        const RoomId& id,
                        const std::string& bundle_path);
bool import_room_bundle(LocalSwmStore& store,
                        const std::string& bundle_path,
                        RoomId& out_id);

}  // namespace sos
