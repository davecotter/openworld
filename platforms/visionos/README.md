# SOS Vision (visionOS) — platform app

Native Vision Pro app. Links `sos-core` via `SOSCoreBridge.mm` and `sos_c_api.h`.

## Toolchain

- Xcode 16+ with visionOS SDK
- Swift 6 + SwiftUI + RealityKit + ARKit
- Swift Package: [MetalSplatter](https://github.com/scier/MetalSplatter) (splat render fallback)

## Create Xcode project (one-time)

1. File → New → Project → visionOS → App
2. Product name: `SOSVisionPro`, bundle `com.openworld.sos.visionpro`
3. Add `../../core/include` to Header Search Paths
4. Link `libsos-core.a` (build with §9.2 in spec-12)
5. Add Objective-C++ bridge file calling `sos_c_api.h`

## MVP screens

- Scan → `sos_capture_session_*`
- Import splat → `sos_import_splat_file`
- View → `sos_load_room` + RealityKit/MetalSplatter overlay

See `.cursor/plans/spec-12-phase-0-implementation-plan.md` §5.
