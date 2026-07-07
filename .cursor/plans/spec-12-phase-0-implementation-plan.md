# SOS Spec Part 12: Phase 0 Implementation Plan (Vision Pro + Quest 3)

## Table of Contents

- [1. Executive Summary](#1-executive-summary)
- [2. Repository Layout](#2-repository-layout)
- [3. Shared Core (`sos-core`)](#3-shared-core-sos-core)
- [4. Toolchain Matrix](#4-toolchain-matrix)
- [5. Vision Pro App Implementation](#5-vision-pro-app-implementation)
- [6. Quest 3 App Implementation](#6-quest-3-app-implementation)
- [7. Cross-Platform SWM Bundle Format](#7-cross-platform-swm-bundle-format)
- [8. Milestones and Task Breakdown](#8-milestones-and-task-breakdown)
- [9. Build Commands](#9-build-commands)
- [10. Risks and Mitigations](#10-risks-and-mitigations)
- [11. References](#11-references)

## 1. Executive Summary

### 1.1 Architecture Decision
1.1.1 **One portable C++20 core** (`sos-core`) owns import normalization, local SWM persistence, and render-scene assembly.  
1.1.2 **Native visionOS app** (Swift + RealityKit + ARKit) for Vision Pro.  
1.1.3 **Native OpenXR Android app** (C++ + Meta OpenXR SDK) for Quest 3.  
1.1.4 Both apps link the same `sos-core` static library via platform build integration.

### 1.2 Why Not Unity-Only for Phase 0
1.2.1 Vision Pro-first quality path is native Apple stack (RealityKit splats, ARKit scene reconstruction, passthrough compositor).  
1.2.2 Quest native OpenXR gives direct passthrough + spatial entity access with fewer abstraction leaks.  
1.2.3 Unity remains a **fallback option for Quest splat rendering** if native Vulkan splat pass slips schedule (see §6.5).

### 1.3 Phase 0 Render Capability by Platform
| Capability | Vision Pro | Quest 3 (initial) | Quest 3 (0b) |
|------------|------------|-------------------|--------------|
| Sensor scan → SWM | Yes (ARKit mesh) | Yes (Scene mesh) | Yes |
| Splat file import → SWM | Yes | Yes | Yes |
| Splat render | Yes (RealityKit / MetalSplatter) | Mesh proxy fallback | Native/Vulkan splat pass |
| Aligned overlay on room | Yes | Yes (passthrough + mesh) | Yes |

## 2. Repository Layout

```text
openworld/
  README.md
  CMakeLists.txt                 # top-level; builds sos-core
  cmake/                         # toolchains (ios, android)
  third_party/                   # pinned deps (json, sqlite amalgamation, glm)
  core/
    include/sos/
      capture_frame.hpp
      import_source.hpp
      normalization.hpp
      local_swm.hpp
      room_record.hpp
      splat_asset.hpp
    src/
      local_swm_store.cpp
      splat_import_ply.cpp
      normalization_pipeline.cpp
      room_bundle_io.cpp
    tests/
  platforms/
    visionos/
      SOSVisionPro/              # Xcode project (Swift app)
      SOSVisionProCoreBridge/    # ObjC++/Swift wrapper over sos-core
    quest/
      android/                   # Gradle + CMake NDK project
        app/src/main/cpp/
        app/src/main/java/
  assets/
    test_splats/                 # sample .ply / .splat for CI
  .cursor/plans/                 # specifications
```

## 3. Shared Core (`sos-core`)

### 3.1 Language and Build
3.1.1 **C++20**, CMake 3.24+, compiled as static library `libsos-core.a`.  
3.1.2 No platform SDK headers inside core (pure portable code).

### 3.2 Required Libraries (Core)
| Library | Version | Purpose | License |
|---------|---------|---------|---------|
| [nlohmann/json](https://github.com/nlohmann/json) | 3.11+ | `room.json`, metadata | MIT |
| [SQLite amalgamation](https://www.sqlite.org/amalgamation.html) | 3.45+ | local index + event log | Public domain |
| [glm](https://github.com/g-truc/glm) | 1.0+ | transforms, quaternions | MIT |
| [xxHash](https://github.com/Cyan4973/xxHash) | 0.8+ | content-addressed asset keys | BSD |
| [Catch2](https://github.com/catchorg/Catch2) | 3.x | unit tests | BSL-1.0 |

### 3.3 Core Modules (Implement in Order)
3.3.1 `room_record` — schema read/write, versioning (`swm_version: 1`).  
3.3.2 `local_swm_store` — directories per spec-10 §5.2, SQLite index, append-only `events.log`.  
3.3.3 `splat_import_ply` — binary PLY + `.splat` reader → canonical `SplatAsset` blob.  
3.3.4 `normalization_pipeline` — unify sensor batches + import results → `ReconstructionDelta`.  
3.3.5 `room_bundle_io` — export/import zip bundle for cross-device manual share.

### 3.4 C API Surface (for Swift/Kotlin JNI bridges)
3.4.1 Expose a thin **C ABI** from C++ (`extern "C"`) to avoid Swift/C++ interop fragility:

```c
// sos_c_api.h (generated/stable)
sos_session_t sos_capture_session_create(const sos_session_config* cfg);
int sos_capture_session_push_frame(sos_session_t, const sos_capture_frame* frame);
int sos_finalize_room(sos_session_t, sos_room_id* out_room_id);

int sos_import_splat_file(const char* path, sos_room_id* out_room_id);
int sos_list_rooms(sos_room_summary* out, int max, int* count);
int sos_load_room(const sos_room_id* id, sos_render_scene* out_scene);
void sos_free_render_scene(sos_render_scene* scene);
```

### 3.5 Canonical Asset Blob (Phase 0)
3.5.1 `assets/{xxhash64}.bin` with header:
```text
magic: "SOSSPLAT"
version: u32
count: u32
fields: pos(3 f32), scale(3 f32), rot(4 f32), color(4 u8), opacity(f32)
```
3.5.2 Optional companion `assets/{hash}_mesh.glb` as render fallback.

## 4. Toolchain Matrix

### 4.1 Host Development Machine (macOS)
4.1.1 macOS 15+ (Apple Silicon recommended).  
4.1.2 **Qt Creator 20** + **Qt 6.11.1** at `/Users/davec/Developer/Qt` (bundled CMake 3.30.5, Ninja).  
4.1.3 **Xcode 16+** with **visionOS SDK** (visionOS 2.x minimum; target visionOS 26/27 features when available).  
4.1.4 **Android Studio Ladybug+** with NDK **r26d** or **r27** (or Qt Creator Android kit once installed).  
4.1.5 Python 3.11+ (fetch scripts).  
4.1.6 Meta **Quest Developer Hub** + device USB/Wi‑Fi debugging enabled.

### 4.4 Qt / Qt Creator (this machine)

**Install root:** `/Users/davec/Developer/Qt`  
**Qt Creator:** `/Users/davec/Developer/Qt/Qt Creator.app` (v20.0.0)  
**Qt macOS kit:** `/Users/davec/Developer/Qt/6.11.1/macos`  
**CMake / Ninja:** `/Users/davec/Developer/Qt/Tools/CMake/...`, `/Users/davec/Developer/Qt/Tools/Ninja/ninja`

#### 4.4.1 What is installed today
| Component | Status |
|-----------|--------|
| Qt 6.11.1 macOS (clang_64) | Installed |
| Qt Creator 20 | Installed |
| CMake 3.30.5 + Ninja | Installed |
| Qt Quick 3D / Qt Quick 3D Xr | **Not installed** (no `QtQuick3D*.framework` under macOS lib) |
| Qt for Android (Quest) | **Not installed** |
| Qt for visionOS (Vision Pro) | **Not installed** |

Add missing kits via **MaintenanceTool.app** — see **§4.4.6** (there is **no** “visionOS” or “Qt Quick 3D Xr” checkbox; Xr ships inside Quick 3D; visionOS requires a source build).

#### 4.4.6 Qt MaintenanceTool install checklist (Qt 6.11.1, fetched 2026-06-29)

Package list pulled from Qt’s online repository cache on this machine (`~/Library/Caches/qt-unified-mac-online/.../Updates.xml`). **Close all MaintenanceTool windows** before CLI install (only one instance allowed).

##### Why you don’t see “vision” or “Xr”

| What you expected | What Qt actually offers |
|-------------------|-------------------------|
| **visionOS / Vision** kit in installer | **Not in MaintenanceTool.** Zero `visionos`/`xros` packages in Qt 6.11.1 online repo. High-immersion Vision Pro requires **building Qt from source** ([Qt visionOS guide](https://doc.qt.io/qt-6.11/qt3dxr-quick-start-guide-applevisionpro.html)). |
| **Qt Quick 3D Xr** separate add-on | **Not a separate package.** Xr APIs (`XrView`, passthrough, anchors) are **part of Qt Quick 3D** — install **Qt Quick 3D** and dependencies. |
| **iOS** in installer | **Yes** — LiDAR/ARKit capture contributor app (RoomPlan/ARKit via native bridge, not Qt Sensors). Separate from Vision Pro high-immersion shell. |

##### Why “Qt Quick 3D” has no Android/macOS sub-checkboxes

The installer often shows **one** “Qt Quick 3D” row under Add-Ons. Platform-specific binaries (`*.android_arm64_v8a`, `*.clang_64`, `*.ios`) are pulled via **AutoDependOn** once you have checked:

1. The **platform kit** first (e.g. **Android ARM64-v8a**, or macOS host already installed)
2. The **parent add-on** (**Qt Quick 3D**, **Qt Shader Tools**, **Qt Quick Timeline**, **Qt Quick 3D Physics**)

You may not see nested rows in the simplified tree. After install, verify files exist:

```bash
ls ~/Developer/Qt/6.11.1/android/lib/libQt6Quick3D*.so
ls ~/Developer/Qt/6.11.1/macos/lib/QtQuick3D*.framework
ls ~/Developer/Qt/6.11.1/ios/lib/libQt6Quick3D*.a   # after iOS kit
```

If missing, use **Advanced** component view or the CLI package list in §4.4.6 CLI block.

##### GUI steps (MaintenanceTool)

1. Quit any open **MaintenanceTool** / Qt Installer windows.
2. Open **`/Users/davec/Developer/Qt/MaintenanceTool.app`**
3. Choose **Add or remove components**
4. Expand **Qt → Qt 6.11.1** (already partially installed)

**For Quest 3 (Qt Quick 3D Xr on Android):**

5. Under **Qt 6.11.1**, check **Android** → **Android ARM64-v8a**  
   - Internal package: `qt.qt6.6111.android_arm64_v8a`  
   - Display name: **Android ARM64-v8a**
6. Under **Add-Ons**, check (single row each — platform binaries auto-selected):
   - **Qt Quick 3D** (`qt.qt6.6111.addons.qtquick3d`)
   - **Qt Shader Tools** (Quick 3D dependency)
   - **Qt Quick Timeline** (Quick 3D dependency)
   - **Qt Quick 3D Physics** (throwables vs scanned room mesh — see §4.4.8)

**For macOS host dev (Qt Creator builds / desktop tools):**

8. Under **Add-Ons**, check **Qt Quick 3D** → **Qt Quick 3D for macOS**  
   - `qt.qt6.6111.addons.qtquick3d` + `qt.qt6.6111.addons.qtquick3d.clang_64`  
   - Also **Qt Shader Tools** + **Qt Quick Timeline** for macOS (`*.clang_64`).

**For iOS capture contributor (LiDAR + GPS — Phase 0/1):**

9. Under **Qt 6.11.1**, check **iOS** (`qt.qt6.6111.ios`)
10. Under **Add-Ons**, **Qt Quick 3D**, **Shader Tools**, **Timeline** (same parent rows; iOS binaries auto-pull)
11. Room scan uses **ARKit / RoomPlan** (native), not Qt Sensors. GPS via **Core Location** (native or Qt Positioning wrapper).

**For Vision Pro high-immersion (Qt Quick 3D Xr — required path on Qt stack):**

12. Under **Qt 6.11.1**, check **Sources** (`qt.qt6.6111.src`) — **no prebuilt visionOS binaries exist anywhere official or third-party** (verified 2026-06-29).
13. Build Qt for visionOS from source (§4.4.7 below). Deploy via **Xcode only**.

14. After Android install: in **Qt Creator → Preferences → Devices → Android**, point SDK/NDK/JDK (Android Studio defaults usually work).

##### CLI install (after closing MaintenanceTool GUI)

```bash
MT="/Users/davec/Developer/Qt/MaintenanceTool.app/Contents/MacOS/MaintenanceTool"

# Quest 3 + macOS Quick 3D (minimal XR set)
"$MT" --accept-licenses --default-answer --confirm-command install \
  qt.qt6.6111.android_arm64_v8a \
  qt.qt6.6111.addons.qtquick3d \
  qt.qt6.6111.addons.qtquick3d.android_arm64_v8a \
  qt.qt6.6111.addons.qtshadertools \
  qt.qt6.6111.addons.qtshadertools.android_arm64_v8a \
  qt.qt6.6111.addons.qtquicktimeline \
  qt.qt6.6111.addons.qtquicktimeline.android_arm64_v8a \
  qt.qt6.6111.addons.qtquick3d.clang_64 \
  qt.qt6.6111.addons.qtshadertools.clang_64 \
  qt.qt6.6111.addons.qtquicktimeline.clang_64

# Optional: iOS capture app
# "$MT" ... install qt.qt6.6111.ios qt.qt6.6111.addons.qtquick3d qt.qt6.6111.addons.qtshadertools qt.qt6.6111.addons.qtquicktimeline

# Required for Vision Pro high-immersion Qt build:
# "$MT" ... install qt.qt6.6111.src
```

Verify install:

```bash
ls /Users/davec/Developer/Qt/6.11.1/android/lib/libQt6Quick3D*.so 2>/dev/null | head
ls /Users/davec/Developer/Qt/6.11.1/macos/lib/QtQuick3D*.framework 2>/dev/null | head
```

##### Alias shortcut (installs many add-ons at once)

MaintenanceTool also advertises alias **`qt6.11.1-full-dev`** (essentials + all add-on dev headers/libs + **Sources**). Heavier download; use if you want everything without ticking individual boxes:

```bash
"$MT" --accept-licenses --default-answer --confirm-command install qt6.11.1-full-dev
```

You still must add **Android ARM64-v8a** (and **iOS** if wanted) platform kits separately — aliases do not replace platform trees.

#### 4.4.7 Build Qt for visionOS from source (high-immersion only)

Prerequisites: `qt.qt6.6111.src` installed (or clone Qt 6.11.1), existing macOS host Qt at `/Users/davec/Developer/Qt/6.11.1/macos`, Xcode with visionOS SDK.

Simulator:

```bash
[QT_SOURCE]/configure -qt-host-path /Users/davec/Developer/Qt/6.11.1/macos \
  -platform macx-visionos-clang -sdk xrsimulator -submodules qtquick3d
cmake --build . --parallel
```

Device:

```bash
[QT_SOURCE]/configure -qt-host-path /Users/davec/Developer/Qt/6.11.1/macos \
  -platform macx-visionos-clang -sdk xros -submodules qtquick3d
cmake --build . --parallel
```

Deploy via **Xcode** (not Qt Creator): `[QT_VISIONOS_BUILD]/bin/qt-cmake -B build -S your_app` → open generated `.xcodeproj`.

See [Getting Started With Apple Vision Pro](https://doc.qt.io/qt-6.11/qt3dxr-quick-start-guide-applevisionpro.html).

**Prebuilt visionOS Qt libraries:** Official Qt ships none. Community builds live in **[qt-visionos-prebuilt](https://github.com/davecotter/qt-visionos-prebuilt)** (local clone: `/Volumes/Developer/git/qt-visionos-prebuilt`) — scripts, manifests, and release tarballs. OpenWorld treats that repo as **source of truth** for visionOS Qt binaries.

#### 4.4.8 Qt modules — SOS product mapping (Phase 0 revised)

| Module | Phase 0? | Role in SOS |
|--------|----------|-------------|
| **Qt Quick 3D** (+ Xr) | **Quest yes**; VP optional | Immersive shell on Quest; VP prefers native RealityKit |
| **Qt Quick 3D Physics** | **Quest yes** | Throwables vs room mesh colliders |
| **Qt Shader Tools / Quick Timeline** | **Quest yes** | Quick 3D dependencies |
| **Qt Multimedia** | **Yes (VP/Quest)** | Local video/audio on virtual screens/speakers — abstract `MediaSource` in core |
| **Qt iOS** | **No** | iOS app is **native Swift** (RoomPlan, Core Location) |
| **Qt Sensors** | **No** | Not head tracking; not room scan — see §4.4.10 |
| **Qt Positioning** | **Phase 1+** | GPS on iOS via **Core Location** in Phase 0 export metadata only |
| **Qt WebView** | **Phase 2+** | Web picture frames / streaming login — see §4.4.9 |
| **Qt Quick Effect Maker** | Optional | Shader authoring tool only |
| **Qt Remote Objects** | Skip | Not needed |
| **AI agent accounts** | Later | Skip for now |

#### 4.4.9 Web picture frames — critique (Phase 2+, spec now)

**Product vision:** Picture frame shows a web page; user logs into Netflix/YouTube in a browser, goes “fullscreen”, content appears on the wall or floating screen in the room.

**Phase 0 decision:** **Do not use Qt WebView.** Ship **local file** playback via `MediaSource` + native/Qt Multimedia.

**Why WebView is hard (later):**

| Issue | Implication |
|-------|-------------|
| **DRM** | Netflix/Disney+ block arbitrary WebView capture; HDCP and EME — often **cannot** texture-map protected video to a 3D quad |
| **ToS / policy** | Scraping or mirroring streaming UI may violate service terms |
| **Input** | XR ray/hand focus vs web page scrolling — needs dedicated browser chrome in 3D |
| **Performance** | Full-page WebView as texture at room scale is GPU-heavy on Quest |
| **visionOS** | WKWebView integration in immersive space has platform constraints |

**Recommended phased approach:**

1. **Phase 0:** `local_file` only — AVFoundation (VP/iOS), Qt Multimedia (Quest).  
2. **Phase 2a:** `url_stream` for **non-DRM** HTTPS video (direct `.mp4`, HLS without DRM).  
3. **Phase 2b:** **Embedded browser object** (Qt WebView / WKWebView) for **non-streaming** pages (photos, dashboards, internal tools).  
4. **Phase 3:** Streaming services — prefer **official APIs / cast protocols** where available; WebView login + fullscreen only where DRM allows; never assume Netflix-in-a-frame works.

**API shape (future-proof):** `MediaSource.source_kind`: `local_file` | `url_stream` | `web_surface (reserved)` | `cast_session (reserved)`.

#### 4.4.10 Head tracking vs Qt Sensors

Latency-free head turning on Vision Pro and Quest is **6DOF device pose** from:

- **Vision Pro:** ARKit + visionOS compositor (IMU + visual-inertial odometry fused by Apple)  
- **Quest:** OpenXR view poses (Meta runtime fusion)  
- **Qt Quick 3D Xr:** consumes runtime pose via `XrView` — **not** `QAccelerometer` / `QGyroscope`

**Qt Sensors** exposes phone/tablet sensors (accel, gyro, magnetometer, light). It does **not** replace XR head tracking. iOS capture app does not need Qt Sensors for scan or for feeding VP/Quest viewers.

**iOS GPS:** write latitude/longitude/accuracy into room bundle export via **Core Location** (native). Qt Positioning optional wrapper in Phase 1+ — not required for Phase 0.

#### 4.4.11 Platform stack summary (Phase 0)

| App | Stack | Qt installer |
|-----|-------|--------------|
| **iOS capture** | Native Swift, RoomPlan, ARKit, Core Location | **Skip Qt iOS** |
| **Vision Pro** | Native Swift, RealityKit, ARKit, AVFoundation media | Optional Qt from **qt-visionos-prebuilt** only if QML shell needed |
| **Quest 3** | Qt Quick 3D Xr + Physics + Multimedia | Android ARM64-v8a + Quick 3D add-ons |
| **Android phone** | Phase 1 | — |

Shared: **`sos-core`** C API for SWM, import, `MediaSource` metadata.

#### 4.4.2 Recommended roles for Qt in Phase 0

| Layer | Use Qt? | Rationale |
|-------|---------|-----------|
| `sos-core` | **No** | Stay SDK-neutral; link from Swift, JNI, or QML via C API only |
| Host build / test | **Yes — Qt Creator** | Open root `CMakeLists.txt`; preset `qt-host-debug` in `CMakePresets.json` |
| macOS dev console (optional) | **Yes — Qt Widgets** | Room list, import picker, splat preview without a headset |
| Quest 3 app | **Optional path B** | [Qt Quick 3D Xr](https://doc.qt.io/qt-6/qt-quick-3d-xr.html) + OpenXR — `XrView`, passthrough, spatial anchors ([Quest quick start](https://doc.qt.io/qt-6/qt3dxr-quest-quick-start.html)) |
| Vision Pro app | **Native Swift/RealityKit primary** | Best splat UX; Qt optional via [qt-visionos-prebuilt](https://github.com/davecotter/qt-visionos-prebuilt) |
| iOS capture app | **Native Swift only** | RoomPlan/LiDAR/GPS — **skip Qt iOS** |

#### 4.4.3 Architecture with Qt Quick 3D Xr (optional unified shell)

```text
┌─────────────────────────────────────────┐
│  QML UI (XrView, room list, import)     │  Qt Quick 3D Xr
├─────────────────────────────────────────┤
│  SOSCoreBridge (C++ / Q_INVOKABLE)      │  thin wrapper
├─────────────────────────────────────────┤
│  sos-core (libsos-core.a)               │  import, SWM, C API
└─────────────────────────────────────────┘
```

Both Quest and Vision Pro can share QML + bridge; platform-specific capture adapters stay in C++ (`ARKitFrameAdapter` vs OpenXR scene mesh).

#### 4.4.4 Qt libraries worth using (when linked — dev or QML shell only)

| Module | Use |
|--------|-----|
| Qt Core | `QStandardPaths`, file I/O, JSON (`QJsonDocument` optional vs nlohmann in core) |
| Qt Sql | Local room index UI (core keeps SQLite directly for portability) |
| Qt Quick 3D Xr | Quest + Vision Pro immersive shell, passthrough, anchors |
| Qt Network | Deferred (federation) |

**Rule:** device builds must not depend on Qt inside `sos-core`; only app/shell targets link Qt.

#### 4.4.5 Open in Qt Creator

1. Launch `/Users/davec/Developer/Qt/Qt Creator.app`
2. **File → Open File or Project…** → `/Volumes/Developer/git/openworld/CMakeLists.txt`
3. Select kit **Qt 6.11.1 for macOS** (or generic CMake with preset)
4. Configure preset **Host Debug (Qt Creator / macOS)**
5. Build → run **sos_core_smoke_test** from Tests pane

References: [Qt Quick 3D Xr](https://doc.qt.io/qt-6.11/qt-quick-3d-xr.html), [Quest 3 getting started](https://doc.qt.io/qt-6.11/qt3dxr-quest-quick-start.html), [Getting started Vision Pro (Qt)](https://doc.qt.io/qt-6/qt3dxr-visionos-quick-start.html).

### 4.2 Vision Pro Target
| Component | Tool / SDK |
|-----------|------------|
| IDE | Xcode |
| Language | Swift 6 |
| UI | SwiftUI |
| 3D / splats | RealityKit (Gaussian splats, visionOS 26+) |
| Capture | ARKit `SceneReconstructionProvider`, `WorldTrackingProvider` |
| Passthrough / immersive | `CompositorLayer`, `ImmersiveSpace` |
| Splat fallback render | [MetalSplatter](https://github.com/scier/MetalSplatter) (Swift/Metal, PLY/.splat) |
| Core bridge | ObjC++ wrapper → `sos_c_api.h` |
| Signing | Apple Developer Program, visionOS device provisioning |

References: [visionOS Get Started](https://developer.apple.com/visionos/get-started/), [RealityKit advances (WWDC26)](https://developer.apple.com/videos/play/wwdc2026/279/).

### 4.3 Quest 3 Target
| Component | Tool / SDK |
|-----------|------------|
| IDE | **Qt Creator** (path B) or Android Studio (path A) |
| Build | Qt CMake Android kit (path B) or Gradle + CMake NDK (path A) |
| Language | C++20 (app), Java/Kotlin (minimal activity shell) |
| XR runtime | **Qt Quick 3D Xr** (path B) or **Khronos OpenXR Loader 1.0.34+** (path A) |
| Meta SDK | Meta XR Core SDK (path A) / bundled via Qt Xr (path B) |
| Required extensions | `XR_KHR_OPENGL_ES_ENABLE`, `XR_FB_passthrough`, `XR_FB_spatial_entity`, Scene extensions |
| Graphics | OpenGL ES 3.2 / Vulkan (prefer Vulkan for splat 0b) |
| Permissions | `com.oculus.permission.USE_ANCHOR_API`, `com.oculus.permission.USE_SCENE_API` |
| Device OS | Horizon OS v62+ |

References: [Meta OpenXR](https://developers.meta.com/horizon/documentation/native/android/mobile-openxr/), [Passthrough](https://developers.meta.com/horizon/documentation/native/android/mobile-passthrough/), [Spatial Anchors](https://developers.meta.com/horizon/documentation/native/android/openxr-lsa-persist-content/).

## 5. Vision Pro App Implementation

### 5.1 App ID and Bundle
5.1.1 Product name: **SOS Vision** (working title).  
5.1.2 Bundle ID pattern: `com.openworld.sos.visionpro`.  
5.1.3 Deployment target: visionOS 2.0+ (raise when using RealityKit splat APIs requiring newer OS).

### 5.2 Xcode Project Structure
```text
SOSVisionPro/
  App/SOSVisionProApp.swift
  UI/MainWindow.swift
  UI/RoomListView.swift
  Capture/CaptureSessionManager.swift
  Capture/ARKitFrameAdapter.swift      # ARKit → sos_capture_frame
  Import/SplatFileImporter.swift       # UIDocumentPicker → sos_import_splat_file
  Render/ImmersiveRoomView.swift       # RealityView / CompositorLayer
  Render/SplatEntityBuilder.swift       # sos_render_scene → RealityKit entities
  Bridge/SOSCoreBridge.mm              # calls sos_c_api
  Bridge/SOSCoreBridge.h
```

### 5.3 Required Apple Frameworks
5.3.1 `SwiftUI`, `RealityKit`, `ARKit`, `CompositorServices` (if custom compositor path).  
5.3.2 `UniformTypeIdentifiers` for splat file picking (`.ply`, custom `.splat`).  
5.3.3 Info.plist: camera/world-sensing usage descriptions.

### 5.4 Capture Pipeline (Vision Pro)
5.4.1 Request **Full Space** immersive mode for scan + view.  
5.4.2 Start `WorldTrackingProvider` + `SceneReconstructionProvider` (mesh classification).  
5.4.3 Each frame:
- read device transform (4×4) → convert to SOS canonical (right-handed, meters)
- optionally sample scene mesh anchors
- pack `sos_capture_frame` → `sos_capture_session_push_frame`
5.4.4 On Stop Scan → `sos_finalize_room` → refresh room list.

### 5.5 Render Pipeline (Vision Pro)
5.5.1 Load room via `sos_load_room`.  
5.5.2 Build RealityKit content:
- **Primary**: RealityKit Gaussian splat component (if asset is splat)
- **Fallback**: MetalSplatter renderer in `RealityView` attachment
- **Mesh-only rooms**: `ModelEntity` from exported mesh proxy
5.5.3 Align using stored room anchor transform + live world tracking.  
5.5.4 Passthrough: use progressive/mixed immersion; synthetic content composited over real world.

### 5.6 Vision Pro MVP UI Screens
5.6.1 **Home**: New Scan | Import Splat | Open Room.  
5.6.2 **Scan**: live status (frames ingested, confidence).  
5.6.3 **View**: Enter immersive space, alignment badge (Aligned / Provisional / Lost).

### 5.7 Vision Pro Build Steps
1. Build `sos-core` for `iphoneos` + `xros` simulator/device slices (see §9).  
2. Open `SOSVisionPro.xcodeproj`, link `libsos-core.a`, add bridge header.  
3. Add MetalSplatter via Swift Package Manager: `https://github.com/scier/MetalSplatter`.  
4. Run on Vision Pro device (simulator limited for ARKit mesh).

## 6. Quest 3 App Implementation

### 6.1 App ID
6.1.1 Package: `com.openworld.sos.quest`.  
6.1.2 Min SDK: API 29; target API 32+ per Meta guidance.

### 6.2 Android Project Structure
```text
quest/android/
  app/build.gradle.kts
  app/src/main/AndroidManifest.xml
  app/src/main/java/com/openworld/sos/MainActivity.java
  app/src/main/cpp/
    main.cpp
    XrApp.cpp / XrApp.h
    XrPassthrough.cpp
    XrSceneMesh.cpp
    SosCoreBridge.cpp          # sos_c_api calls
    render/MeshOverlayRenderer.cpp
  app/src/main/assets/
```

### 6.3 Required SDK Packages
6.3.1 Download **Meta XR Core SDK** from [Meta Horizon downloads](https://developers.meta.com/horizon/downloads/).  
6.3.2 Use **Khronos OpenXR SDK** `OpenXR-SDK` (loader 1.0.34+) as submodule or prefab.  
6.3.3 Start from Meta sample **`XrPassthrough`** + **`XrSpatialAnchor`** (merge into one app).

### 6.4 Capture Pipeline (Quest 3)
6.4.1 Enable Scene API + spatial entities (manifest permissions).  
6.4.2 Query room mesh / plane anchors via `XR_FB_scene_*` extensions.  
6.4.3 Convert anchor transforms + mesh buffers → `sos_capture_frame` batches (lower frequency than VP; 2–5 Hz mesh snapshots).  
6.4.4 Mark frames with `sensor_mode = inferred` when raw camera not available.

### 6.5 Render Pipeline (Quest 3)
6.5.1 **Phase 0a (ship first)**:
- Passthrough layer (`XR_FB_passthrough`) as base
- Draw mesh proxy from SWM aligned via spatial anchor / stored transform
- Splat rooms: render coarse mesh proxy or point cloud generated at import time in `sos-core`

6.5.2 **Phase 0b (fast follow)** — choose one:
- **Native**: Vulkan compute splat sort + draw (port ideas from [GaussianSplatViewer](https://github.com/zachdrouin/GaussianSplatViewer))
- **Pragmatic fallback**: separate Unity 6 Quest module linked only for splat viewing (higher maintenance)

6.5.3 Relocalization: create/load spatial anchor at room save time; on view, resolve anchor and apply stored `transform_world`.

### 6.6 Quest MVP UI
6.6.1 Laser-pointer menu (OpenXR composition layer UI or simple quad).  
6.6.2 Buttons: Scan, Import, Save, View, Room List.  
6.6.3 File picker via Android Storage Access Framework for splat import.

### 6.7 Quest Build Steps
1. Install Quest Developer Hub; enable developer mode on headset.  
2. Build `sos-core` for `android-arm64-v8a` NDK.  
3. Configure `externalNativeBuild` cmake in Gradle to link `sos-core`.  
4. Copy Meta OpenXR loader + headers into `app/src/main/cpp/third_party/openxr`.  
5. `./gradlew :app:assembleDebug` → install via `adb install`.

## 7. Cross-Platform SWM Bundle Format

### 7.1 Bundle File
7.1.1 Extension: `.sosroom` (zip container).  
7.1.2 Contents:
```text
manifest.json          # swm_version, room_id, created_at, source_type
room.json
events.log
assets/*
```

### 7.2 Export/Import API
7.2.1 `sos_export_room_bundle(room_id, path)`  
7.2.2 `sos_import_room_bundle(path, room_id_out)`  
7.2.3 Both platforms expose Share sheet / file picker around these calls.

## 8. Milestones and Task Breakdown

### M0 — Core Foundation (Week 1–2)
- [x] CMake project + root README
- [x] `local_swm_store` read/write + smoke test
- [ ] fetch third_party deps (json, sqlite, glm, xxHash)
- [ ] PLY/.splat import → canonical blob (parser)
- [x] C API (`sos_c_api.h`) + smoke test
- [ ] `.sosroom` bundle export/import

### M1 — Vision Pro Vertical Slice (Week 3–5)
- [ ] Xcode project + core bridge (Swift sources in `platforms/visionos/SOSVisionPro/`)
- [ ] ARKit capture adapter
- [ ] Scan → save → list rooms
- [ ] Immersive view with mesh/splat render
- [ ] Splat file import UI
- [ ] Device test: alignment acceptable in one room

### M2 — Quest 3 Vertical Slice (Week 4–6, overlaps M1)
- [ ] OpenXR app shell from Meta samples (Gradle stub in `platforms/quest/android/`)
- [ ] Passthrough + mesh overlay render
- [ ] Scene capture → sos-core save
- [ ] View saved room with anchor relocalization
- [ ] Splat import (mesh proxy view)

### M3 — Cross-Platform Proof (Week 7)
- [ ] Export `.sosroom` on Vision Pro → import on Quest
- [ ] Document alignment limitations + bootstrap scan flow
- [ ] Performance pass (frame time, load time)

## 9. Build Commands

### 9.0 Qt Creator (macOS host — recommended)

1. Open `/Volumes/Developer/git/openworld/CMakeLists.txt` in `/Users/davec/Developer/Qt/Qt Creator.app`
2. Use CMake preset **`qt-host-debug`** (see `CMakePresets.json`)
3. Build project; run test **`sos_core_smoke`**

CLI equivalent:

```bash
cd /Volumes/Developer/git/openworld
/Users/davec/Developer/Qt/Tools/CMake/CMake.app/Contents/bin/cmake --preset qt-host-debug
/Users/davec/Developer/Qt/Tools/CMake/CMake.app/Contents/bin/cmake --build build/host
/Users/davec/Developer/Qt/Tools/CMake/CMake.app/Contents/bin/ctest --test-dir build/host
```

### 9.1 Core (macOS host)
```bash
cd /Volumes/Developer/git/openworld
cmake -B build/host -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host
ctest --test-dir build/host
```

### 9.2 Core (visionOS device)
```bash
cmake -B build/xros -DCMAKE_SYSTEM_NAME=visionOS \
  -DCMAKE_OSX_SYSROOT=xros \
  -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build/xros
```

### 9.3 Core (Quest Android arm64)
```bash
export ANDROID_NDK=$HOME/Library/Android/sdk/ndk/26.3.11579264
cmake -B build/quest -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29
cmake --build build/quest
```

### 9.4 Vision Pro app
```bash
xcodebuild -project platforms/visionos/SOSVisionPro/SOSVisionPro.xcodeproj \
  -scheme SOSVisionPro -destination 'platform=visionOS' build
```

### 9.5 Quest app
```bash
cd platforms/quest/android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

### 9.6 Qt Quick 3D Xr apps (after installing Android + visionOS + Quick3D kits)

Create from Qt Creator template **Qt Quick 3D Xr Application**, then link `libsos-core.a` and add `SOSCoreBridge` QML plugin. See §4.4.3.

```bash
# Example: configure with Qt Android kit (paths vary by kit name in Creator)
/Users/davec/Developer/Qt/Tools/CMake/CMake.app/Contents/bin/cmake \
  -B build/quest-qt -DCMAKE_PREFIX_PATH=/Users/davec/Developer/Qt/6.11.1/android \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a
```

## 10. Risks and Mitigations

| Risk | Mitigation |
|------|------------|
| Quest native splat render is hard | **Required:** custom GLES/Vulkan splat pass on Quest; no mesh-only fallback |
| Cross-device anchor alignment | Manual bundle + bootstrap rescan on Quest |
| ARKit mesh ≠ splat quality | Allow splat import path as primary quality source |
| OpenXR extension fragmentation | Capability probe at startup; degrade features gracefully |
| Large splat memory | Chunk assets in sos-core; LOD decimation on import |

## 12. Implementation Status (Phase 0 — 2026-07-07)

| Component | Path | Status |
|-----------|------|--------|
| `sos-core` + SOSSPLAT + bundle I/O | `core/` | **Done** — host + xros-sim + ios builds, tests pass |
| C API incl. `sos_read_asset` | `core/include/sos/sos_c_api.h` | **Done** |
| Vision Pro app + RealityKit/Metal splats | `platforms/visionos/` | Sources + Xcode project; build pending simulator fix |
| Quest GLES splat renderer | `platforms/quest/android/` | **Implemented**; APK build blocked (no NDK) |
| iOS capture app | `platforms/ios/SOSCapture/` | **Scaffold** |
| Parity + splat catalog policy | `.cursor/plans/platform-parity-matrix.md`, `scripts/splat_sources_sync.sh` | **Done** |

## 11. References

- Qt Quick 3D Xr: https://doc.qt.io/qt-6.11/qt-quick-3d-xr.html
- Qt Quest 3: https://doc.qt.io/qt-6.11/qt3dxr-quest-quick-start.html
- Qt visionOS: https://doc.qt.io/qt-6/qt3dxr-visionos-quick-start.html
- Apple visionOS: https://developer.apple.com/visionos/get-started/
- RealityKit / Gaussian splats (WWDC26): https://developer.apple.com/videos/play/wwdc2026/279/
- MetalSplatter: https://github.com/scier/MetalSplatter
- Meta OpenXR: https://developers.meta.com/horizon/documentation/native/android/mobile-openxr/
- Meta Passthrough: https://developers.meta.com/horizon/documentation/native/android/mobile-passthrough/
- Quest GaussianSplatViewer (Unity reference): https://github.com/zachdrouin/GaussianSplatViewer
- Scaniverse: https://dev.scaniverse.com/
- Phase 0 product spec: `spec-10-mvp-phase-0-local-room.md`
