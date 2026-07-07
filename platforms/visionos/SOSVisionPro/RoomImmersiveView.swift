import Metal
import RealityKit
import SwiftUI

/// Full Space immersive view with passthrough compositing (visionOS 27 mixed immersion).
struct RoomImmersiveView: View {
  @Environment(AppModel.self) private var appModel

  var body: some View {
    RealityView { content in
      guard let roomId = appModel.activeRoomId,
            let scene = SOSCoreBridge.loadRenderScene(roomId: roomId),
            let assetData = SOSCoreBridge.readAsset(hash: scene.primaryAssetHash) else { return }

      let entity = SplatEntityBuilder.makeEntity(from: assetData, device: MTLCreateSystemDefaultDevice())
      content.add(entity)
      appModel.alignmentStatus = scene.hasSplatAsset ? "Aligned (Splat)" : "Aligned (Scan)"
    }
    .gesture(TapGesture().onEnded { appModel.alignmentStatus = "Relocalized" })
  }
}
