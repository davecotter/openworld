#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SOS_ROOM_ID_MAX 64

typedef struct sos_room_id {
  char id[SOS_ROOM_ID_MAX];
} sos_room_id;

typedef struct sos_room_summary {
  char id[SOS_ROOM_ID_MAX];
  char display_name[128];
  char source_type[32];
  int64_t updated_at_ns;
} sos_room_summary;

typedef struct sos_render_scene {
  char room_json[65536];
  char primary_asset_hash[128];
  char asset_kind[32];
  int32_t has_splat_asset;
  int32_t has_mesh_proxy;
} sos_render_scene;

typedef void* sos_session_t;

typedef struct sos_session_config {
  const char* platform;       /* "visionos" | "quest" | "ios" */
  const char* store_root;     /* path to SOSLocal root */
  const char* display_name;
} sos_session_config;

typedef struct sos_capture_frame {
  int64_t timestamp_ns;
  float pose_world_from_device[7];
  float pose_confidence;
  float tracking_confidence;
} sos_capture_frame;

typedef struct sos_gps_tag {
  double latitude;
  double longitude;
  float accuracy_m;
  int has_gps;
} sos_gps_tag;

sos_session_t sos_capture_session_create(const sos_session_config* cfg);
void sos_capture_session_destroy(sos_session_t session);
int sos_capture_session_push_frame(sos_session_t session, const sos_capture_frame* frame);
int sos_finalize_room(sos_session_t session, sos_room_id* out_room_id);

int sos_import_splat_file(const char* store_root, const char* path, sos_room_id* out_room_id);

int sos_list_rooms(const char* store_root, sos_room_summary* out, int max, int* count);
int sos_load_room(const char* store_root, const sos_room_id* id, sos_render_scene* out_scene);
int sos_read_asset(const char* store_root,
                   const char* content_hash,
                   uint8_t* out_bytes,
                   size_t out_capacity,
                   size_t* out_size);
void sos_free_render_scene(sos_render_scene* scene);

int sos_export_room_bundle(const char* store_root, const sos_room_id* id, const char* bundle_path);
int sos_import_room_bundle(const char* store_root, const char* bundle_path, sos_room_id* out_id);
int sos_set_room_gps(const char* store_root, const sos_room_id* id, const sos_gps_tag* gps);

#ifdef __cplusplus
}
#endif
