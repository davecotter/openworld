#include "sos/sos_c_api.h"

#include <cstring>
#include <memory>
#include <string>

#include "sos/local_swm.hpp"
#include "sos/normalization_pipeline.hpp"
#include "sos/room_record.hpp"

namespace {

using sos::LocalSwmStore;
using sos::NormalizationPipeline;
using sos::RoomId;
using sos::RoomRecord;
using sos::room_record_from_json;
using sos::room_record_to_json;

struct Session {
  sos_session_config cfg{};
  NormalizationPipeline pipeline;
  std::unique_ptr<LocalSwmStore> store;
};

bool WriteDelta(LocalSwmStore& store, const sos::ReconstructionDelta& delta) {
  const auto json = room_record_to_json(delta.record);
  if (!store.write_room(delta.room_id, json, delta.assets)) {
    return false;
  }
  store.append_event(delta.room_id, "room_finalized");
  return true;
}

void FillRenderScene(const RoomRecord& record, sos_render_scene* out_scene) {
  const auto json = room_record_to_json(record);
  std::strncpy(out_scene->room_json, json.c_str(), sizeof(out_scene->room_json) - 1);
  std::strncpy(out_scene->primary_asset_hash, record.primary_asset_hash.c_str(),
               sizeof(out_scene->primary_asset_hash) - 1);
  std::strncpy(out_scene->asset_kind, record.asset_kind.c_str(), sizeof(out_scene->asset_kind) - 1);
  out_scene->has_splat_asset = record.asset_kind == "splat_chunk" ? 1 : 0;
  out_scene->has_mesh_proxy = record.asset_kind == "mesh_proxy" ? 1 : 0;
}

}  // namespace

extern "C" {

sos_session_t sos_capture_session_create(const sos_session_config* cfg) {
  if (!cfg || !cfg->store_root) {
    return nullptr;
  }
  auto* session = new Session();
  session->cfg = *cfg;
  session->store = std::make_unique<LocalSwmStore>(cfg->store_root);
  return session;
}

void sos_capture_session_destroy(sos_session_t session) {
  delete static_cast<Session*>(session);
}

int sos_capture_session_push_frame(sos_session_t session, const sos_capture_frame* frame) {
  if (!session || !frame) {
    return -1;
  }
  auto* s = static_cast<Session*>(session);
  sos::CaptureFrame cf{};
  cf.timestamp_ns = frame->timestamp_ns;
  std::memcpy(cf.pose_world_from_device, frame->pose_world_from_device,
              sizeof(cf.pose_world_from_device));
  cf.pose_confidence = frame->pose_confidence;
  cf.tracking_confidence = frame->tracking_confidence;
  cf.depth_available = false;
  std::strncpy(cf.platform, s->cfg.platform ? s->cfg.platform : "unknown", sizeof(cf.platform) - 1);
  s->pipeline.ingest_frame(cf);
  return 0;
}

int sos_finalize_room(sos_session_t session, sos_room_id* out_room_id) {
  if (!session || !out_room_id) {
    return -1;
  }
  auto* s = static_cast<Session*>(session);
  const char* name = s->cfg.display_name ? s->cfg.display_name : "Room";
  const auto delta = s->pipeline.finalize_room(name);
  if (delta.room_id.id[0] == '\0') {
    return -2;
  }
  if (!WriteDelta(*s->store, delta)) {
    return -3;
  }
  std::strncpy(out_room_id->id, delta.room_id.id, SOS_ROOM_ID_MAX - 1);
  return 0;
}

int sos_import_splat_file(const char* store_root, const char* path, sos_room_id* out_room_id) {
  if (!store_root || !path || !out_room_id) {
    return -1;
  }
  NormalizationPipeline pipeline;
  const auto delta = pipeline.ingest_splat_file(path);
  if (delta.room_id.id[0] == '\0') {
    return -2;
  }
  LocalSwmStore store(store_root);
  if (!WriteDelta(store, delta)) {
    return -3;
  }
  std::strncpy(out_room_id->id, delta.room_id.id, SOS_ROOM_ID_MAX - 1);
  return 0;
}

int sos_list_rooms(const char* store_root, sos_room_summary* out, int max, int* count) {
  if (!store_root || !out || !count || max <= 0) {
    return -1;
  }
  LocalSwmStore store(store_root);
  const auto rooms = store.list_rooms();
  *count = static_cast<int>(rooms.size() < static_cast<size_t>(max) ? rooms.size()
                                                                   : static_cast<size_t>(max));
  for (int i = 0; i < *count; ++i) {
    out[i] = sos_room_summary{};
    std::strncpy(out[i].id, rooms[static_cast<size_t>(i)].id, SOS_ROOM_ID_MAX - 1);
    std::strncpy(out[i].display_name, rooms[static_cast<size_t>(i)].display_name, 127);
    std::strncpy(out[i].source_type, rooms[static_cast<size_t>(i)].source_type, 31);
    out[i].updated_at_ns = rooms[static_cast<size_t>(i)].updated_at_ns;
  }
  return 0;
}

int sos_load_room(const char* store_root, const sos_room_id* id, sos_render_scene* out_scene) {
  if (!store_root || !id || !out_scene) {
    return -1;
  }
  LocalSwmStore store(store_root);
  RoomId rid{};
  std::strncpy(rid.id, id->id, sizeof(rid.id) - 1);
  std::string json;
  if (!store.read_room_json(rid, json)) {
    return -2;
  }
  RoomRecord record;
  if (!room_record_from_json(json, record)) {
    return -3;
  }
  std::memset(out_scene, 0, sizeof(*out_scene));
  FillRenderScene(record, out_scene);
  return 0;
}

int sos_read_asset(const char* store_root,
                   const char* content_hash,
                   uint8_t* out_bytes,
                   size_t out_capacity,
                   size_t* out_size) {
  if (!store_root || !content_hash || !out_size) {
    return -1;
  }
  LocalSwmStore store(store_root);
  std::vector<uint8_t> bytes;
  if (!store.read_asset(content_hash, bytes)) {
    return -2;
  }
  *out_size = bytes.size();
  if (out_bytes && out_capacity >= bytes.size()) {
    std::memcpy(out_bytes, bytes.data(), bytes.size());
    return 0;
  }
  return bytes.empty() ? -2 : -3;
}

void sos_free_render_scene(sos_render_scene* scene) {
  if (scene) {
    std::memset(scene, 0, sizeof(*scene));
  }
}

int sos_export_room_bundle(const char* store_root, const sos_room_id* id, const char* bundle_path) {
  if (!store_root || !id || !bundle_path) {
    return -1;
  }
  LocalSwmStore store(store_root);
  RoomId rid{};
  std::strncpy(rid.id, id->id, sizeof(rid.id) - 1);
  return store.export_bundle(rid, bundle_path) ? 0 : -2;
}

int sos_import_room_bundle(const char* store_root, const char* bundle_path, sos_room_id* out_id) {
  if (!store_root || !bundle_path || !out_id) {
    return -1;
  }
  LocalSwmStore store(store_root);
  RoomId rid{};
  if (!store.import_bundle(bundle_path, rid)) {
    return -2;
  }
  std::strncpy(out_id->id, rid.id, SOS_ROOM_ID_MAX - 1);
  return 0;
}

int sos_set_room_gps(const char* store_root, const sos_room_id* id, const sos_gps_tag* gps) {
  if (!store_root || !id || !gps) {
    return -1;
  }
  LocalSwmStore store(store_root);
  RoomId rid{};
  std::strncpy(rid.id, id->id, sizeof(rid.id) - 1);
  std::string json;
  if (!store.read_room_json(rid, json)) {
    return -2;
  }
  RoomRecord record;
  if (!room_record_from_json(json, record)) {
    return -3;
  }
  record.has_gps = gps->has_gps != 0;
  record.gps_latitude = gps->latitude;
  record.gps_longitude = gps->longitude;
  record.gps_accuracy_m = gps->accuracy_m;
  if (!store.write_room(rid, room_record_to_json(record), {})) {
    return -4;
  }
  return 0;
}

}  // extern "C"
