# Platform Feature Parity Matrix (Phase 0)

**Hard rule:** equivalent user-facing capability on every device unless hardware makes it impossible. Native APIs on one platform → OSS equivalent elsewhere (MIT/BSD/Apache/LGPL-compatible only).

| Feature | Vision Pro | Quest 3 | iOS Capture | Host |
|---------|------------|---------|-------------|------|
| Splat import (.ply/.splat) | ✅ `sos_import_splat_file` | ✅ same core | ✅ bridge | ✅ core tests |
| Splat render | ✅ RealityKit Gaussian (vOS 26+/27) + Metal instanced fallback | ✅ custom GLES3 splat pass | N/A (capture only) | ⚠️ decode tests only |
| Room scan | ✅ ARKit WorldTracking + SceneReconstruction | ⚠️ OpenXR scene stub (Phase 0 shell) | ✅ RoomPlan stub + pose frame | N/A |
| Mesh proxy (alignment) | ✅ sensor finalize | ✅ same core asset | ✅ same core | ✅ tests |
| Local SWM save/list | ✅ | ✅ | ✅ | ✅ |
| `.sosroom` bundle export/import | ✅ | ✅ core API | ✅ | ✅ tests |
| Passthrough + overlay | ✅ ImmersiveSpace mixed/progressive | ✅ EGL transparent + splat draw | N/A | N/A |
| GPS metadata | — | — | ✅ `sos_set_room_gps` | — |

## Renderer stack (license-safe)

| Platform | Primary | Fallback | License |
|----------|---------|----------|---------|
| visionOS 27 | RealityKit native Gaussian splats | Metal instanced quads (`SplatEntityBuilder`) | Apple SDK + MIT patterns |
| Quest 3 | `GlesSplatRenderer` (sort + instanced draw) | — | Project code (Apache-style project license) |
| Core | SOSSPLAT canonical blob | `sos_read_asset` | Project code |

## Quarterly splat catalog

See `.cursor/rules/gaussian-splat-sources.mdc` and `scripts/splat_sources_sync.sh`.
