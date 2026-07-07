import Foundation

enum SOSCaptureBridge {
  static func startScan(displayName: String) -> String? {
    SOSCaptureBridgeObjC.startScan(withDisplayName: displayName)
  }

  static func importSplat(path: String) -> String? {
    SOSCaptureBridgeObjC.importSplat(atPath: path)
  }

  static func exportLatestBundle() -> String? {
    SOSCaptureBridgeObjC.exportLatestBundle()
  }
}
