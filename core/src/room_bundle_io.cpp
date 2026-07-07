#include "sos/local_swm.hpp"

#include "sos/room_record.hpp"
#include "sos/zip_io.hpp"

namespace sos {

bool export_room_bundle(const LocalSwmStore& store,
                        const RoomId& id,
                        const std::string& bundle_path) {
  std::string room_json;
  if (!store.read_room_json(id, room_json)) {
    return false;
  }

  RoomRecord record;
  if (!room_record_from_json(room_json, record)) {
    return false;
  }

  std::vector<ZipEntry> entries;
  entries.push_back({"manifest.json",
                     std::vector<uint8_t>(std::string("{\"swm_version\":1,\"room_id\":\"" +
                                                      record.id + "\",\"source_type\":\"" +
                                                      record.source_type + "\"}")
                                              .begin(),
                                      std::string("{\"swm_version\":1,\"room_id\":\"" +
                                                  record.id + "\",\"source_type\":\"" +
                                                  record.source_type + "\"}")
                                              .end())});
  entries.push_back({"room.json", std::vector<uint8_t>(room_json.begin(), room_json.end())});

  std::string events;
  store.read_events(id, events);
  entries.push_back({"events.log", std::vector<uint8_t>(events.begin(), events.end())});

  if (!record.primary_asset_hash.empty()) {
    std::vector<uint8_t> asset;
    if (store.read_asset(record.primary_asset_hash, asset)) {
      entries.push_back({"assets/" + record.primary_asset_hash + ".bin",
                         std::move(asset)});
    }
  }

  return zip_write_store(bundle_path, entries);
}

bool import_room_bundle(LocalSwmStore& store,
                        const std::string& bundle_path,
                        RoomId& out_id) {
  std::vector<ZipEntry> entries;
  if (!zip_read_store(bundle_path, entries)) {
    return false;
  }

  std::string room_json;
  std::string events;
  std::vector<std::pair<std::string, std::vector<uint8_t>>> assets;

  for (const auto& entry : entries) {
    if (entry.name == "room.json") {
      room_json.assign(entry.data.begin(), entry.data.end());
    } else if (entry.name == "events.log") {
      events.assign(entry.data.begin(), entry.data.end());
    } else if (entry.name.rfind("assets/", 0) == 0) {
      const auto filename = entry.name.substr(7);
      const auto dot = filename.find(".bin");
      const auto hash = dot == std::string::npos ? filename : filename.substr(0, dot);
      assets.emplace_back(hash, entry.data);
    }
  }

  if (room_json.empty()) {
    return false;
  }

  RoomRecord record;
  if (!room_record_from_json(room_json, record)) {
    return false;
  }

  RoomId id{};
  record.id.copy(id.id, sizeof(id.id) - 1);
  if (!store.write_room(id, room_json, assets)) {
    return false;
  }
  if (!events.empty()) {
    store.write_events(id, events);
  }
  out_id = id;
  return true;
}

}  // namespace sos
