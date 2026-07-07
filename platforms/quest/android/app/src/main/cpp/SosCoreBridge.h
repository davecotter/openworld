#pragma once

#include <cstddef>
#include <cstdint>

void sos_bridge_set_store_root(const char* path);
bool sos_bridge_import_splat(const char* path, char* out_room_id, size_t out_len);
bool sos_bridge_load_splat_asset(const char* room_id,
                                 uint8_t* out_bytes,
                                 size_t out_capacity,
                                 size_t* out_size);
