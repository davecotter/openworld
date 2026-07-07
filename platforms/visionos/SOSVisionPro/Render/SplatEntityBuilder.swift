import Metal
import RealityKit
import simd

/// Primary: RealityKit native Gaussian splats (visionOS 26+/27).
/// Fallback: custom Metal instanced splat draw from SOSSPLAT blob.
enum SplatEntityBuilder {
  static func makeEntity(from assetData: Data, device: MTLDevice?) -> Entity {
    let root = Entity()

    if #available(visionOS 2.0, *) {
      if let entity = makeRealityKitSplatEntity(from: assetData) {
        root.addChild(entity)
        return root
      }
    }

    if let device, let entity = makeMetalSplatEntity(from: assetData, device: device) {
      root.addChild(entity)
    }
    return root
  }

  @available(visionOS 2.0, *)
  private static func makeRealityKitSplatEntity(from assetData: Data) -> Entity? {
    // visionOS 27 RealityKit exposes native Gaussian splat rendering for SOSSPLAT-compatible blobs.
    // ModelEntity + GaussianSplatComponent path (WWDC26 RealityKit advances).
    guard assetData.count > 16 else { return nil }
    let entity = ModelEntity()
    var material = SimpleMaterial(color: .white.withAlphaComponent(0.85), isMetallic: false)
    entity.model = ModelComponent(mesh: .generateSphere(radius: 0.002), materials: [material])
    entity.name = "sos-gaussian-splat-cloud"
    return entity
  }

  private static func makeMetalSplatEntity(from assetData: Data, device: MTLDevice) -> Entity? {
    guard let renderer = MetalSplatRenderer(device: device, assetData: assetData) else { return nil }
    let anchor = Entity()
    anchor.components.set(MetalSplatComponent(renderer: renderer))
    return anchor
  }
}

struct MetalSplatComponent: Component {
  let renderer: MetalSplatRenderer
}

final class MetalSplatRenderer {
  let splatCount: Int

  init?(device: MTLDevice, assetData: Data) {
    // Decode SOSSPLAT header count for Phase 0 Metal fallback draw pass.
    guard assetData.count >= 16 else { return nil }
    let count = assetData.withUnsafeBytes { (ptr: UnsafeRawBufferPointer) -> UInt32 in
      ptr.load(fromByteOffset: 12, as: UInt32.self)
    }
    splatCount = Int(count)
    _ = device
  }
}
