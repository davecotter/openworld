# SOS Quest (Quest 3) — OpenXR Android app

Native Quest app using Khronos OpenXR + Meta XR Core SDK. Links `sos-core` via JNI/CMake.

## Toolchain

- Android Studio + NDK r26+
- Khronos OpenXR Loader **1.0.34+**
- Meta XR Core SDK (from [Meta Horizon downloads](https://developers.meta.com/horizon/downloads/))
- Horizon OS v62+ on Quest 3

## Bootstrap from Meta samples

1. Copy structure from `XrPassthrough` + `XrSpatialAnchor` samples
2. Add `SosCoreBridge.cpp` calling `sos_c_api.h`
3. CMake `externalNativeBuild` links `libsos-core.a` for `arm64-v8a`

## Required manifest permissions

```xml
<uses-permission android:name="com.oculus.permission.USE_ANCHOR_API" />
<uses-permission android:name="com.oculus.permission.USE_SCENE_API" />
```

## MVP flow

- Scene mesh capture → `sos_capture_session_*`
- Passthrough + mesh overlay render
- Splat import → view mesh proxy (splat render in Phase 0b)

See `.cursor/plans/spec-12-phase-0-implementation-plan.md` §6.
