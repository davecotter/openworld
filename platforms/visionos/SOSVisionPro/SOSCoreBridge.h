#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface SOSRoomSummary : NSObject
@property(nonatomic, copy) NSString* roomId;
@property(nonatomic, copy) NSString* displayName;
@property(nonatomic, copy) NSString* sourceType;
@property(nonatomic) int64_t updatedAtNs;
@end

@interface SOSCaptureFrame : NSObject
@property(nonatomic) int64_t timestampNs;
@property(nonatomic, copy) NSArray<NSNumber*>* pose;
@property(nonatomic) float poseConfidence;
@property(nonatomic) float trackingConfidence;
@end

@interface SOSRenderScene : NSObject
@property(nonatomic, copy) NSString* roomJSON;
@property(nonatomic, copy) NSString* primaryAssetHash;
@property(nonatomic, copy) NSString* assetKind;
@property(nonatomic) BOOL hasSplatAsset;
@property(nonatomic) BOOL hasMeshProxy;
@end

@interface SOSCoreBridgeObjC : NSObject
+ (NSArray<SOSRoomSummary*>*)listRoomsAtStoreRoot:(NSString*)storeRoot;
+ (void*)createSessionWithPlatform:(NSString*)platform storeRoot:(NSString*)storeRoot displayName:(NSString*)displayName;
+ (void)destroySession:(void*)session;
+ (BOOL)pushFrame:(SOSCaptureFrame*)frame session:(void*)session;
+ (nullable NSString*)finalizeSession:(void*)session;
+ (nullable NSString*)importSplatAtPath:(NSString*)path storeRoot:(NSString*)storeRoot;
+ (nullable SOSRenderScene*)loadSceneForRoomId:(NSString*)roomId storeRoot:(NSString*)storeRoot;
+ (nullable NSData*)readAssetWithHash:(NSString*)hash storeRoot:(NSString*)storeRoot;
+ (BOOL)exportBundleForRoomId:(NSString*)roomId storeRoot:(NSString*)storeRoot path:(NSString*)path;
+ (nullable NSString*)importBundleAtPath:(NSString*)path storeRoot:(NSString*)storeRoot;
@end

NS_ASSUME_NONNULL_END
