# OpenWorld SOS — Phase 0 Complete

Spatial Operating System MVP with **first-class Gaussian splat rendering on all platforms**.

**Repository:** https://github.com/davecotter/openworld

## Policies

- **Feature parity:** all platforms ship Gaussian splat viewing (see `.cursor/plans/platform-parity-matrix.md`)
- **OSS fallback:** Quest uses custom GLES splat renderer; Vision Pro uses RealityKit + Metal fallback
- **License gate:** MIT/BSD/Apache/LGPL-compatible only for shared OSS deps
- **Quarterly splat catalog:** `.cursor/rules/gaussian-splat-sources.mdc` + `scripts/splat_sources_sync.sh`

## Build core (host)

```bash
cd /Volumes/Developer/git/openworld
DEVELOPER_DIR="/Applications/Xcode-27.0.0-Beta.2.app/Contents/Developer" \
  /Users/davec/Developer/Qt/Tools/CMake/CMake.app/Contents/bin/cmake --preset qt-host-debug
/Users/davec/Developer/Qt/Tools/CMake/CMake.app/Contents/bin/cmake --build build/host
/Users/davec/Developer/Qt/Tools/CMake/CMake.app/Contents/bin/ctest --test-dir build/host
```

## Platform apps

| App | Path | Splat renderer |
|-----|------|----------------|
| **SOS Vision** | `platforms/visionos/` | RealityKit Gaussian splats (visionOS 26+/27) + Metal instanced fallback |
| **SOS Quest** | `platforms/quest/android/` | Custom OpenGL ES 3.2 splat pass (sort + instanced quads) |
| **SOS Capture** | `platforms/ios/SOSCapture/` | Export `.sosroom` bundles with splat assets |

## visionOS 27 features (SOS Vision)

- Native **RealityKit Gaussian splat** rendering (WWDC26/27)
- **ARKit** `WorldTrackingProvider` + `SceneReconstructionProvider` capture
- **ImmersiveSpace** with `.mixed` / `.progressive` passthrough compositing
- **Physical space lighting** ready (RealityKit 27 point/spot lights on scanned mesh)
- Bundle export/import for cross-device `.sosroom` sharing

## Cross-platform core

- Canonical **SOSSPLAT** blob (`core/include/sos/splat_asset.hpp`)
- PLY / `.splat` import → content-addressed assets
- `sos_read_asset()` C API for renderer consumption
- `.sosroom` zip bundles (manifest + room.json + assets + events.log)

See `.cursor/plans/spec-12-phase-0-implementation-plan.md` for full specification.
