#include <cassert>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "sos/sos_c_api.h"
#include "sos/splat_asset.hpp"
#include "sos/splat_decode.hpp"

namespace {

std::filesystem::path WriteTestSplat(const std::filesystem::path& dir) {
  std::vector<sos::CanonicalSplat> splats(3);
  splats[0].pos[0] = 0.f;
  splats[0].pos[1] = 1.f;
  splats[0].pos[2] = 0.f;
  splats[0].scale[0] = splats[0].scale[1] = splats[0].scale[2] = 0.05f;
  splats[0].opacity = 1.f;
  splats[1].pos[0] = 1.f;
  splats[1].scale[0] = splats[1].scale[1] = splats[1].scale[2] = 0.05f;
  splats[2].pos[2] = -1.f;
  splats[2].scale[0] = splats[2].scale[1] = splats[2].scale[2] = 0.05f;

  const auto blob = sos::encode_splat_blob(splats);
  const auto path = dir / "test.splat";
  std::ofstream out(path, std::ios::binary);
  for (const auto& s : splats) {
    out.write(reinterpret_cast<const char*>(s.pos), sizeof(float) * 3);
    out.write(reinterpret_cast<const char*>(s.scale), sizeof(float) * 3);
    uint8_t rgba[4] = {200, 200, 200, 255};
    out.write(reinterpret_cast<const char*>(rgba), 4);
    uint8_t rot[4] = {128, 128, 128, 128};
    out.write(reinterpret_cast<const char*>(rot), 4);
  }
  (void)blob;
  return path;
}

}  // namespace

int main() {
  const auto tmp = std::filesystem::temp_directory_path() / "sos_test_store";
  std::filesystem::remove_all(tmp);
  std::filesystem::create_directories(tmp);

  sos_session_config cfg{};
  cfg.platform = "host";
  const std::string store_root = tmp.string();
  cfg.store_root = store_root.c_str();
  cfg.display_name = "Test Room";

  sos_session_t session = sos_capture_session_create(&cfg);
  assert(session != nullptr);

  sos_capture_frame frame{};
  frame.timestamp_ns = 1;
  frame.pose_world_from_device[0] = 0.f;
  frame.pose_world_from_device[1] = 1.6f;
  frame.pose_world_from_device[2] = 0.f;
  frame.pose_world_from_device[6] = 1.f;
  frame.pose_confidence = 0.9f;
  frame.tracking_confidence = 0.9f;
  assert(sos_capture_session_push_frame(session, &frame) == 0);

  sos_room_id room_id{};
  assert(sos_finalize_room(session, &room_id) == 0);
  assert(room_id.id[0] != '\0');

  sos_room_summary summaries[8]{};
  int count = 0;
  assert(sos_list_rooms(cfg.store_root, summaries, 8, &count) == 0);
  assert(count >= 1);

  sos_render_scene scene{};
  assert(sos_load_room(cfg.store_root, &room_id, &scene) == 0);
  assert(scene.room_json[0] != '\0');
  assert(scene.has_mesh_proxy == 1);

  sos_capture_session_destroy(session);

  const auto splat_path = WriteTestSplat(tmp);
  sos_room_id splat_room{};
  assert(sos_import_splat_file(store_root.c_str(), splat_path.string().c_str(), &splat_room) == 0);

  sos_render_scene splat_scene{};
  assert(sos_load_room(store_root.c_str(), &splat_room, &splat_scene) == 0);
  assert(splat_scene.has_splat_asset == 1);
  assert(splat_scene.primary_asset_hash[0] != '\0');

  size_t asset_size = 0;
  assert(sos_read_asset(store_root.c_str(), splat_scene.primary_asset_hash, nullptr, 0, &asset_size) == -3);
  assert(asset_size > 0);
  std::vector<uint8_t> asset(asset_size);
  assert(sos_read_asset(store_root.c_str(), splat_scene.primary_asset_hash, asset.data(), asset.size(),
                        &asset_size) == 0);

  std::vector<sos::SplatInstance> instances;
  assert(sos::decode_splat_instances(asset, instances));
  assert(instances.size() == 3);

  const auto bundle_path = (tmp / "room.sosroom").string();
  assert(sos_export_room_bundle(store_root.c_str(), &splat_room, bundle_path.c_str()) == 0);

  sos_room_id imported{};
  const auto import_store = (tmp / "imported").string();
  std::filesystem::create_directories(import_store);
  assert(sos_import_room_bundle(import_store.c_str(), bundle_path.c_str(), &imported) == 0);
  assert(imported.id[0] != '\0');

  std::printf("sos_core_smoke_test ok\n");
  return 0;
}
