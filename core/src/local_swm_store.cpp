#include "sos/local_swm.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "sos/room_record.hpp"

namespace fs = std::filesystem;

namespace sos {

namespace {

std::string RoomDir(const std::string& root, const std::string& id) {
  return root + "/rooms/" + id;
}

std::string AssetsDir(const std::string& root) {
  return root + "/assets";
}

std::string IndexPath(const std::string& root) {
  return root + "/index.json";
}

bool WriteFile(const fs::path& path, const std::string& data) {
  std::error_code ec;
  fs::create_directories(path.parent_path(), ec);
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    return false;
  }
  out.write(data.data(), static_cast<std::streamsize>(data.size()));
  return out.good();
}

bool WriteBytes(const fs::path& path, const std::vector<uint8_t>& data) {
  std::error_code ec;
  fs::create_directories(path.parent_path(), ec);
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    return false;
  }
  out.write(reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(data.size()));
  return out.good();
}

}  // namespace

LocalSwmStore::LocalSwmStore(std::string root_path) : root_path_(std::move(root_path)) {
  fs::create_directories(root_path_ + "/rooms");
  fs::create_directories(AssetsDir(root_path_));
  rebuild_index();
}

bool LocalSwmStore::write_room(
    const RoomId& id,
    const std::string& room_json,
    const std::vector<std::pair<std::string, std::vector<uint8_t>>>& assets) {
  const auto room_dir = RoomDir(root_path_, id.id);
  fs::create_directories(room_dir);
  if (!WriteFile(room_dir + "/room.json", room_json)) {
    return false;
  }
  for (const auto& [hash, bytes] : assets) {
    if (!WriteBytes(AssetsDir(root_path_) + "/" + hash + ".bin", bytes)) {
      return false;
    }
  }
  append_event(id, "room_written");
  rebuild_index();
  return true;
}

bool LocalSwmStore::read_room_json(const RoomId& id, std::string& out_json) const {
  const auto path = RoomDir(root_path_, id.id) + "/room.json";
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return false;
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  out_json = ss.str();
  return true;
}

bool LocalSwmStore::read_asset(const std::string& content_hash,
                               std::vector<uint8_t>& out_bytes) const {
  const auto path = AssetsDir(root_path_) + "/" + content_hash + ".bin";
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return false;
  }
  out_bytes.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  return true;
}

bool LocalSwmStore::read_events(const RoomId& id, std::string& out_events) const {
  const auto path = RoomDir(root_path_, id.id) + "/events.log";
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    out_events.clear();
    return true;
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  out_events = ss.str();
  return true;
}

bool LocalSwmStore::write_events(const RoomId& id, const std::string& events) {
  return WriteFile(RoomDir(root_path_, id.id) + "/events.log", events);
}

bool LocalSwmStore::append_event(const RoomId& id, const std::string& line) {
  const auto path = RoomDir(root_path_, id.id) + "/events.log";
  std::ofstream out(path, std::ios::app);
  if (!out) {
    fs::create_directories(RoomDir(root_path_, id.id));
    out.open(path, std::ios::app);
  }
  if (!out) {
    return false;
  }
  out << line << "\n";
  return out.good();
}

std::vector<RoomSummary> LocalSwmStore::list_rooms() const {
  std::vector<RoomSummary> result;
  const fs::path rooms_dir = root_path_ + "/rooms";
  if (!fs::exists(rooms_dir)) {
    return result;
  }
  for (const auto& entry : fs::directory_iterator(rooms_dir)) {
    if (!entry.is_directory()) {
      continue;
    }
    RoomSummary summary{};
    const auto id = entry.path().filename().string();
    id.copy(summary.id, sizeof(summary.id) - 1);

    std::string json;
    RoomId rid{};
    id.copy(rid.id, sizeof(rid.id) - 1);
    if (read_room_json(rid, json)) {
      RoomRecord record;
      if (room_record_from_json(json, record)) {
        record.display_name.copy(summary.display_name, sizeof(summary.display_name) - 1);
        record.source_type.copy(summary.source_type, sizeof(summary.source_type) - 1);
        summary.updated_at_ns = record.updated_at_ns;
      } else {
        id.copy(summary.display_name, sizeof(summary.display_name) - 1);
        std::string source = "unknown";
        source.copy(summary.source_type, sizeof(summary.source_type) - 1);
      }
    } else {
      id.copy(summary.display_name, sizeof(summary.display_name) - 1);
      std::string source = "unknown";
      source.copy(summary.source_type, sizeof(summary.source_type) - 1);
    }
    result.push_back(summary);
  }
  return result;
}

void LocalSwmStore::rebuild_index() const {
  std::ostringstream ss;
  ss << "{\"rooms\":[";
  bool first = true;
  const auto rooms = const_cast<LocalSwmStore*>(this)->list_rooms();
  for (const auto& room : rooms) {
    if (!first) {
      ss << ",";
    }
    first = false;
    ss << "{\"id\":\"" << room.id << "\",\"display_name\":\"" << room.display_name << "\"}";
  }
  ss << "]}";
  WriteFile(IndexPath(root_path_), ss.str());
}

bool LocalSwmStore::export_bundle(const RoomId& id, const std::string& bundle_path) const {
  return export_room_bundle(*this, id, bundle_path);
}

bool LocalSwmStore::import_bundle(const std::string& bundle_path, RoomId& out_id) {
  return import_room_bundle(*this, bundle_path, out_id);
}

}  // namespace sos
