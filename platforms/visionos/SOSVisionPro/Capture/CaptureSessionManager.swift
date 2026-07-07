import ARKit
import Foundation
import QuartzCore
import simd

/// visionOS 27: ARKit WorldTracking + SceneReconstruction for room capture.
@MainActor
final class CaptureSessionManager: ObservableObject {
  @Published var isScanning = false
  @Published var frameCount = 0
  @Published var alignmentStatus = "Idle"

  private var sessionHandle: UnsafeMutableRawPointer?
  private let storeRoot: String
  private let displayName: String
  private var worldTracking: WorldTrackingProvider?
  private var sceneReconstruction: SceneReconstructionProvider?
  private var arSession = ARKitSession()

  init(storeRoot: String, displayName: String) {
    self.storeRoot = storeRoot
    self.displayName = displayName
  }

  func startScan() async {
    guard !isScanning else { return }
    sessionHandle = SOSCoreBridge.createSession(displayName: displayName)
    frameCount = 0
    alignmentStatus = "Scanning"

    worldTracking = WorldTrackingProvider()
    sceneReconstruction = SceneReconstructionProvider()
    do {
      try await arSession.run([worldTracking!, sceneReconstruction!])
      isScanning = true
    } catch {
      alignmentStatus = "ARKit unavailable"
    }
  }

  func ingestCurrentFrame() {
    guard isScanning, let handle = sessionHandle, let tracking = worldTracking else { return }
    guard let deviceAnchor = tracking.queryDeviceAnchor(atTimestamp: CACurrentMediaTime()) else { return }

    let t = deviceAnchor.originFromAnchorTransform
    let frame = SOSCaptureFramePayload(
      timestampNs: Int64(CACurrentMediaTime() * 1_000_000_000),
      pose: [
        t.columns.3.x, t.columns.3.y, t.columns.3.z,
        0, 0, 0, 1
      ],
      poseConfidence: 0.9,
      trackingConfidence: 0.9
    )
    if SOSCoreBridge.pushFrame(frame, session: handle) {
      frameCount += 1
    }
    if sceneReconstruction != nil {
      alignmentStatus = "Mesh + tracking"
    }
  }

  func stopAndSave() -> String? {
    defer {
      if let handle = sessionHandle {
        SOSCoreBridge.destroySession(handle)
        sessionHandle = nil
      }
      isScanning = false
      arSession.stop()
      alignmentStatus = "Saved"
    }
    guard let handle = sessionHandle else { return nil }
    return SOSCoreBridge.finalizeSession(handle)
  }
}