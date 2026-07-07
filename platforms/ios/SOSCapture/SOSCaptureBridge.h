#import <Foundation/Foundation.h>

@interface SOSCaptureBridgeObjC : NSObject
+ (NSString*)storeRoot;
+ (nullable NSString*)startScanWithDisplayName:(NSString*)name;
+ (nullable NSString*)importSplatAtPath:(NSString*)path;
+ (nullable NSString*)exportLatestBundle;
@end
