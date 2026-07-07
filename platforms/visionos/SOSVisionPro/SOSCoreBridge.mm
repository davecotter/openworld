#import "SOSCoreBridge.h"
#include "sos/sos_c_api.h"

#include <stdlib.h>
#include <string.h>

static void CopyNSStringToRoomId(NSString* src, sos_room_id* dst) {
  const char* cstr = [src UTF8String];
  if (!cstr) {
    dst->id[0] = '\0';
    return;
  }
  strncpy(dst->id, cstr, SOS_ROOM_ID_MAX - 1);
  dst->id[SOS_ROOM_ID_MAX - 1] = '\0';
}

@implementation SOSRoomSummary
@end

@implementation SOSCaptureFrame
@end

@implementation SOSRenderScene
@end

@implementation SOSCoreBridgeObjC

+ (NSArray<SOSRoomSummary*>*)listRoomsAtStoreRoot:(NSString*)storeRoot {
  sos_room_summary summaries[64];
  int count = 0;
  if (sos_list_rooms([storeRoot UTF8String], summaries, 64, &count) != 0) {
    return @[];
  }
  NSMutableArray* result = [NSMutableArray arrayWithCapacity:count];
  for (int i = 0; i < count; ++i) {
    SOSRoomSummary* item = [[SOSRoomSummary alloc] init];
    item.roomId = [NSString stringWithUTF8String:summaries[i].id] ?: @"";
    item.displayName = [NSString stringWithUTF8String:summaries[i].display_name] ?: item.roomId;
    item.sourceType = [NSString stringWithUTF8String:summaries[i].source_type] ?: @"unknown";
    item.updatedAtNs = summaries[i].updated_at_ns;
    [result addObject:item];
  }
  return result;
}

+ (void*)createSessionWithPlatform:(NSString*)platform
                         storeRoot:(NSString*)storeRoot
                       displayName:(NSString*)displayName {
  sos_session_config cfg{};
  cfg.platform = [platform UTF8String];
  cfg.store_root = [storeRoot UTF8String];
  cfg.display_name = [displayName UTF8String];
  return sos_capture_session_create(&cfg);
}

+ (void)destroySession:(void*)session {
  sos_capture_session_destroy(session);
}

+ (BOOL)pushFrame:(SOSCaptureFrame*)frame session:(void*)session {
  if (!frame || !session) {
    return NO;
  }
  sos_capture_frame cf{};
  cf.timestamp_ns = frame.timestampNs;
  cf.pose_confidence = frame.poseConfidence;
  cf.tracking_confidence = frame.trackingConfidence;
  for (int i = 0; i < 7; ++i) {
    cf.pose_world_from_device[i] = i < (int)frame.pose.count ? frame.pose[i].floatValue : 0.f;
  }
  return sos_capture_session_push_frame(session, &cf) == 0;
}

+ (NSString*)finalizeSession:(void*)session {
  sos_room_id room{};
  if (sos_finalize_room(session, &room) != 0) {
    return nil;
  }
  return [NSString stringWithUTF8String:room.id];
}

+ (NSString*)importSplatAtPath:(NSString*)path storeRoot:(NSString*)storeRoot {
  sos_room_id room{};
  if (sos_import_splat_file([storeRoot UTF8String], [path UTF8String], &room) != 0) {
    return nil;
  }
  return [NSString stringWithUTF8String:room.id];
}

+ (SOSRenderScene*)loadSceneForRoomId:(NSString*)roomId storeRoot:(NSString*)storeRoot {
  sos_room_id id{};
  CopyNSStringToRoomId(roomId, &id);
  sos_render_scene scene{};
  if (sos_load_room([storeRoot UTF8String], &id, &scene) != 0) {
    return nil;
  }
  SOSRenderScene* payload = [[SOSRenderScene alloc] init];
  payload.roomJSON = [NSString stringWithUTF8String:scene.room_json] ?: @"";
  payload.primaryAssetHash = [NSString stringWithUTF8String:scene.primary_asset_hash] ?: @"";
  payload.assetKind = [NSString stringWithUTF8String:scene.asset_kind] ?: @"";
  payload.hasSplatAsset = scene.has_splat_asset != 0;
  payload.hasMeshProxy = scene.has_mesh_proxy != 0;
  return payload;
}

+ (NSData*)readAssetWithHash:(NSString*)hash storeRoot:(NSString*)storeRoot {
  size_t size = 0;
  if (sos_read_asset([storeRoot UTF8String], [hash UTF8String], NULL, 0, &size) != -3 || size == 0) {
    return nil;
  }
  NSMutableData* data = [NSMutableData dataWithLength:size];
  if (sos_read_asset([storeRoot UTF8String], [hash UTF8String], (uint8_t*)data.mutableBytes, size, &size) != 0) {
    return nil;
  }
  return data;
}

+ (BOOL)exportBundleForRoomId:(NSString*)roomId
                    storeRoot:(NSString*)storeRoot
                         path:(NSString*)path {
  sos_room_id id{};
  CopyNSStringToRoomId(roomId, &id);
  return sos_export_room_bundle([storeRoot UTF8String], &id, [path UTF8String]) == 0;
}

+ (NSString*)importBundleAtPath:(NSString*)path storeRoot:(NSString*)storeRoot {
  sos_room_id room{};
  if (sos_import_room_bundle([storeRoot UTF8String], [path UTF8String], &room) != 0) {
    return nil;
  }
  return [NSString stringWithUTF8String:room.id];
}

@end
