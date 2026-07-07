#import "SOSCaptureBridge.h"
#include "sos/sos_c_api.h"

@implementation SOSCaptureBridgeObjC

+ (NSString*)storeRoot {
  NSURL* base = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory
                                                         inDomain:NSUserDomainMask
                                                appropriateForURL:nil
                                                           create:YES
                                                            error:nil];
  NSURL* dir = [base URLByAppendingPathComponent:@"SOSLocal" isDirectory:YES];
  [[NSFileManager defaultManager] createDirectoryAtURL:dir withIntermediateDirectories:YES attributes:nil error:nil];
  return dir.path;
}

+ (NSString*)startScanWithDisplayName:(NSString*)name {
  sos_session_config cfg{};
  cfg.platform = "ios";
  cfg.store_root = [[self storeRoot] UTF8String];
  cfg.display_name = [name UTF8String];
  sos_session_t session = sos_capture_session_create(&cfg);
  if (!session) {
    return nil;
  }
  sos_capture_frame frame{};
  frame.timestamp_ns = (int64_t)([[NSDate date] timeIntervalSince1970] * 1000000000.0);
  frame.pose_world_from_device[6] = 1.f;
  frame.pose_confidence = 0.8f;
  frame.tracking_confidence = 0.8f;
  sos_capture_session_push_frame(session, &frame);
  sos_room_id room{};
  if (sos_finalize_room(session, &room) != 0) {
    sos_capture_session_destroy(session);
    return nil;
  }
  sos_capture_session_destroy(session);
  return [NSString stringWithUTF8String:room.id];
}

+ (NSString*)importSplatAtPath:(NSString*)path {
  sos_room_id room{};
  if (sos_import_splat_file([[self storeRoot] UTF8String], [path UTF8String], &room) != 0) {
    return nil;
  }
  return [NSString stringWithUTF8String:room.id];
}

+ (NSString*)exportLatestBundle {
  sos_room_summary summaries[8];
  int count = 0;
  if (sos_list_rooms([[self storeRoot] UTF8String], summaries, 8, &count) != 0 || count <= 0) {
    return nil;
  }
  sos_room_id id{};
  strncpy(id.id, summaries[0].id, SOS_ROOM_ID_MAX - 1);
  NSString* out = [[self storeRoot] stringByAppendingPathComponent:[NSString stringWithFormat:@"%s.sosroom", summaries[0].id]];
  if (sos_export_room_bundle([[self storeRoot] UTF8String], &id, [out UTF8String]) != 0) {
    return nil;
  }
  return out;
}

@end
