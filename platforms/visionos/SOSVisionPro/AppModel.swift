import Foundation
import UniformTypeIdentifiers

@Observable
final class AppModel {
  var rooms: [RoomListItem] = []
  var activeRoomId: String?
  var alignmentStatus = "Idle"
  var captureManager: CaptureSessionManager?

  func refreshRooms() {
    rooms = SOSCoreBridge.listRooms()
  }

  func beginScan() {
    Task { @MainActor in
      captureManager = CaptureSessionManager(storeRoot: SOSCoreBridge.storeRoot(), displayName: "Scanned Room")
      await captureManager?.startScan()
    }
  }

  func stopScanAndSave() {
    Task { @MainActor in
      if let roomId = captureManager?.stopAndSave() {
        activeRoomId = roomId
        refreshRooms()
      }
    }
  }
}

struct RoomListItem: Identifiable {
  let id: String
  let displayName: String
  let sourceType: String
}
