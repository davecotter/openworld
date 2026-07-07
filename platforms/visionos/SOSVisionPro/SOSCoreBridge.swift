import Foundation

enum SOSCoreBridge {
  static func storeRoot() -> String {
    let base = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first!
    let dir = base.appendingPathComponent("SOSLocal", isDirectory: true)
    try? FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
    return dir.path
  }

  static func listRooms() -> [RoomListItem] {
    SOSCoreBridgeObjC.listRooms(atStoreRoot: storeRoot()).map {
      RoomListItem(id: $0.roomId, displayName: $0.displayName, sourceType: $0.sourceType)
    }
  }

  static func createSession(displayName: String) -> UnsafeMutableRawPointer? {
    SOSCoreBridgeObjC.createSession(withPlatform: "visionos", storeRoot: storeRoot(), displayName: displayName)
  }

  static func destroySession(_ session: UnsafeMutableRawPointer) {
    SOSCoreBridgeObjC.destroySession(session)
  }

  static func pushFrame(_ frame: SOSCaptureFramePayload, session: UnsafeMutableRawPointer) -> Bool {
    let payload = SOSCaptureFrame()
    payload.timestampNs = frame.timestampNs
    payload.poseConfidence = frame.poseConfidence
    payload.trackingConfidence = frame.trackingConfidence
    payload.pose = frame.pose.map { NSNumber(value: $0) }
    return SOSCoreBridgeObjC.push(payload, session: session)
  }

  static func finalizeSession(_ session: UnsafeMutableRawPointer) -> String? {
    SOSCoreBridgeObjC.finalizeSession(session)
  }

  static func importSplat(path: String) -> String? {
    SOSCoreBridgeObjC.importSplat(atPath: path, storeRoot: storeRoot())
  }

  static func loadRenderScene(roomId: String) -> RenderScenePayload? {
    guard let scene = SOSCoreBridgeObjC.loadScene(forRoomId: roomId, storeRoot: storeRoot()) else { return nil }
    return RenderScenePayload(
      roomJSON: scene.roomJSON,
      primaryAssetHash: scene.primaryAssetHash,
      assetKind: scene.assetKind,
      hasSplatAsset: scene.hasSplatAsset,
      hasMeshProxy: scene.hasMeshProxy
    )
  }

  static func readAsset(hash: String) -> Data? {
    SOSCoreBridgeObjC.readAsset(withHash: hash, storeRoot: storeRoot())
  }

  static func exportBundle(roomId: String, path: String) -> Bool {
    SOSCoreBridgeObjC.exportBundle(forRoomId: roomId, storeRoot: storeRoot(), path: path)
  }

  static func importBundle(path: String) -> String? {
    SOSCoreBridgeObjC.importBundle(atPath: path, storeRoot: storeRoot())
  }
}

struct RenderScenePayload {
  let roomJSON: String
  let primaryAssetHash: String
  let assetKind: String
  let hasSplatAsset: Bool
  let hasMeshProxy: Bool
}

struct SOSCaptureFramePayload {
  let timestampNs: Int64
  let pose: [Float]
  let poseConfidence: Float
  let trackingConfidence: Float
}
