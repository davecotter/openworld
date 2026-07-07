#include "SosCoreBridge.h"

#include <cstring>
#include <string>

#include "sos/sos_c_api.h"

namespace {

std::string g_store_root;

}  // namespace

void sos_bridge_set_store_root(const char* path) {
  g_store_root = path ? path : "";
}

bool sos_bridge_import_splat(const char* path, char* out_room_id, size_t out_len) {
  if (!path || !out_room_id || out_len == 0 || g_store_root.empty()) {
    return false;
  }
  sos_room_id room{};
  if (sos_import_splat_file(g_store_root.c_str(), path, &room) != 0) {
    return false;
  }
  std::strncpy(out_room_id, room.id, out_len - 1);
  out_room_id[out_len - 1] = '\0';
  return true;
}

bool sos_bridge_load_splat_asset(const char* room_id,
                                 uint8_t* out_bytes,
                                 size_t out_capacity,
                                 size_t* out_size) {
  if (!room_id || !out_size || g_store_root.empty()) {
    return false;
  }
  sos_room_id id{};
  std::strncpy(id.id, room_id, SOS_ROOM_ID_MAX - 1);
  sos_render_scene scene{};
  if (sos_load_room(g_store_root.c_str(), &id, &scene) != 0) {
    return false;
  }
  const int rc = sos_read_asset(g_store_root.c_str(), scene.primary_asset_hash, out_bytes, out_capacity, out_size);
  return rc == 0 || rc == -3;
}
