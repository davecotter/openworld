# SOS Spec Part 10: MVP Phase 0 — Local Room (iOS + Vision Pro + Quest 3)

## Table of Contents

- [1. First Goal Statement](#1-first-goal-statement)
- [2. Scope: In and Out](#2-scope-in-and-out)
- [3. Build Order](#3-build-order)
- [4. Platform-Agnostic Import Abstraction](#4-platform-agnostic-import-abstraction)
- [5. Local SWM (On-Device Node)](#5-local-swm-on-device-node)
- [6. Minimum iOS App (LiDAR Capture)](#6-minimum-ios-app-lidar-capture)
- [7. Minimum Vision Pro App](#7-minimum-vision-pro-app)
- [8. Minimum Quest 3 App](#8-minimum-quest-3-app)
- [9. Shared Media Playback (Phase 0)](#9-shared-media-playback-phase-0)
- [10. End-to-End User Flows](#10-end-to-end-user-flows)
- [11. Acceptance Criteria](#11-acceptance-criteria)
- [12. Deferred to Phase 1+](#12-deferred-to-phase-1)

## 1. First Goal Statement

### 1.1 Primary Objective
1.1.1 Build a **platform-agnostic import abstraction** that ingests room models from **live sensors** or **existing Gaussian splat files**, normalizes them into SOS canonical form, and persists them in a **local SWM**.

### 1.2 Client Objective
1.2.1 Ship **minimum viable apps** on **iOS (LiDAR capture)**, **Apple Vision Pro**, and **Meta Quest 3** that can:
- import/scan with sensors (platform-appropriate)
- import existing splat files
- **write** to local SWM
- **read** from local SWM
- **render** the local node model **over the real-world room** (aligned overlay / mixed)
- play **local media** on virtual screens/speakers (shared viewing / audio — see §9)

### 1.3 Success Definition
1.3.1 A user scans or imports a room on one device, saves to local SWM, relaunches (or switches device), reads the same local SWM, and sees the model aligned over the physical room. iOS contributes high-quality LiDAR scans + GPS-tagged room bundles; Vision Pro and Quest view and host shared media.

## 2. Scope: In and Out

### 2.1 In Scope (Phase 0)
2.1.1 Shared core library: import abstraction + local SWM read/write + normalized asset pipeline.  
2.1.2 Sensor-based capture adapters (**iOS** RoomPlan/ARKit, **Vision Pro** ARKit mesh, **Quest 3** Scene mesh).  
2.1.3 Splat file import adapter (generic Gaussian splat + Scaniverse path when available).  
2.1.4 Local-only persistence (no federation, no remote mesh required).  
2.1.5 Real-world-aligned rendering over scanned room (head tracking via **platform XR APIs**, not Qt Sensors).  
2.1.6 **Shared media playback** — local video/audio files on virtual screens and speakers (abstract API; URL/streaming services deferred).  
2.1.7 Single-room focus (one active room node per local SWM instance).

### 2.2 Out of Scope (Phase 0)
2.2.1 Multi-room ghost alignment and doorway graph reconciliation.  
2.2.2 BLE/mDNS mesh discovery and WAN federation.  
2.2.3 Remote avatar viewing of distant spaces.  
2.2.4 Virtual object ownership/handoff (Part 9 product rules; basic placed media objects OK in Phase 0).  
2.2.5 Full replacement safety certification (basic downgrade rules only).  
2.2.6 **Generic Android** client (Phase 1).  
2.2.7 **Qt WebView / in-frame web pages** (Netflix, YouTube login flows — Phase 2+; see Part 12 §4.4.9).  
2.2.8 **Qt Positioning / geospatial federation** (GPS tag on iOS export OK; full Location stack Phase 1+).  
2.2.9 MCP server (optional debug hook only).

## 3. Build Order

### 3.1 Step 1 — Shared Core (Platform Agnostic)
3.1.1 Define import abstraction interfaces.  
3.1.2 Implement local SWM store (SQLite + content-addressed asset files, or equivalent).  
3.1.3 Implement normalization pipeline (sensor observations and splat imports → canonical assets + room entity).  
3.1.4 Implement local SWM read API and render-scene builder.

### 3.2 Step 2 — iOS Minimum App (Capture)
3.2.1 **Native Swift** app: RoomPlan/ARKit LiDAR scan → `sos-core` import abstraction.  
3.2.2 Attach **GPS** (Core Location) to room bundle metadata on export.  
3.2.3 Export `.sosroom` for import on Vision Pro / Quest.  
3.2.4 **Do not require Qt for iOS** — native APIs for scan quality and simplicity.

### 3.3 Step 3 — Vision Pro Minimum App
3.3.1 **Native Swift + RealityKit + ARKit** for view/scan/splat render (best UX).  
3.3.2 Wire Vision Pro adapter to shared `sos-core` via C API bridge.  
3.3.3 Optional future: Qt Quick 3D Xr using [qt-visionos-prebuilt](https://github.com/davecotter/qt-visionos-prebuilt) if shared QML outweighs native splat path.  
3.3.4 Validate scan → save → reload → render loop on device.

### 3.4 Step 4 — Quest 3 Minimum App
3.4.1 Qt Quick 3D Xr **or** native OpenXR — wire Quest adapter to same import abstraction.  
3.4.2 Wire Quest render backend for real-world-aligned overlay.  
3.4.3 Validate same local SWM bundle can be copied and opened on Quest.

### 3.5 Step 5 — Splat Import Path
3.5.1 Add file picker / sideload import for existing splat packages.  
3.5.2 Normalize to canonical `SplatAsset`, save to local SWM, render on all viewer apps.

### 3.6 Step 6 — Shared Media (Phase 0)
3.6.1 Define `MediaSource` abstraction in core (local file Phase 0; `url_stream` reserved).  
3.6.2 Virtual **video screen** + **audio speaker** objects play local files in-room (shared watch/listen).

## 4. Platform-Agnostic Import Abstraction

### 4.1 Core Interfaces
4.1.1 All platforms MUST implement against these contracts (language-agnostic; C++/Rust/Swift/Kotlin bindings as needed):

```text
ImportSource
  - start(session_config) -> SessionHandle
  - stop(session_handle)
  - poll_frames(session_handle) -> CaptureFrame[]

SplatImportSource
  - import_file(path_or_uri) -> ImportResult
  - import_stream(chunks) -> ImportResult

NormalizationPipeline
  - ingest_frames(frames[]) -> ReconstructionDelta
  - ingest_import(import_result) -> ReconstructionDelta
  - finalize_room(session_id) -> RoomRecord

LocalSwmStore
  - write_room(room_record, assets[])
  - read_room(room_id) -> RoomRecord
  - read_assets(room_id) -> AssetStream
  - list_rooms() -> RoomSummary[]
```

### 4.2 Import Source Types
4.2.1 `sensor_live` — platform camera/depth/IMU/SLAM stream.  
4.2.2 `splat_file` — existing Gaussian splat on disk.  
4.2.3 `scaniverse_package` — Scaniverse export (treated as splat/mesh import with provenance tag).

### 4.3 Normalization Output
4.3.1 Every import path MUST produce:
- one `RoomRecord` (metadata, transform, confidence, provenance)
- one or more canonical assets (mesh and/or splat chunks)
- optional pose graph snapshot for relocalization

### 4.4 Canonical Asset Format (Phase 0 Minimum)
4.4.1 Container: content-addressed blobs under local store root.  
4.4.2 Required asset kinds:
- `splat_chunk` (canonical splat parameters)
- `mesh_proxy` (fallback for non-splat renderers)
- `metadata_json` (room + provenance)

4.4.3 Phase 0 MAY use a simplified single-file splat bundle internally; public contract is still "normalized local SWM asset refs."

## 5. Local SWM (On-Device Node)

### 5.1 Phase 0 Topology
5.1.1 Local SWM runs **inside the app process** or as an embedded library-backed store (not a separate appliance VM yet).

### 5.2 Storage Layout
5.2.1 Recommended on-device layout:
```text
~/SOSLocal/
  rooms/{room_id}/room.json
  rooms/{room_id}/events.log
  assets/{content_hash}.bin
  index.json
```

### 5.3 Read/Write API Semantics
5.3.1 **Write**: append observation/reconstruction events; materialize current room state.  
5.3.2 **Read**: load room entity + asset refs + latest transform for render alignment.  
5.3.3 **Relocalization**: on render startup, match live tracking to stored room anchor (platform adapter responsibility).

### 5.4 Local Node Identity
5.4.1 Phase 0 uses a single implicit `local_node_id` per device install.  
5.4.2 Cross-device sharing in Phase 0 is manual export/import of room bundle (file share), not live sync.

## 6. Minimum iOS App (LiDAR Capture)

### 6.1 Required Features
6.1.1 **Scan mode**: RoomPlan/ARKit LiDAR → normalization → save/export local SWM bundle.  
6.1.2 **Import mode**: splat file → normalization → save.  
6.1.3 **Export mode**: write `.sosroom` with optional **GPS** metadata (Core Location).  
6.1.4 **No in-headset XR view required** on iOS Phase 0 (capture contributor device).

### 6.2 Platform Stack
6.2.1 **Native Swift** — RoomPlan, ARKit, Core Location.  
6.2.2 **`sos-core` bridge** — same C API as other platforms.  
6.2.3 **Skip Qt for iOS** for the SOS app (Qt iOS kit not needed unless building a separate Qt Quick shell).

### 6.3 Head Tracking Note
6.3.1 Low-latency head motion on Vision Pro / Quest comes from **ARKit / visionOS compositor / OpenXR** 6DOF pose (IMU + visual fusion at OS level).  
6.3.2 **Qt Sensors is not the head-tracking API** — it exposes phone-grade motion sensors, not XR device pose. Do not use for scene reorientation in XR viewers.

## 7. Minimum Vision Pro App

### 7.1 Required Features
7.1.1 **Scan mode**: live sensor import → normalization → save to local SWM.  
7.1.2 **Import mode**: load splat file → normalization → save to local SWM.  
7.1.3 **View mode**: read local SWM → render aligned over real room (**high immersion only**).  
7.1.4 **Room picker**: list saved local rooms, select active room.  
7.1.5 **Media**: play local video/audio on placed virtual screens/speakers (§9).

### 7.2 Platform Stack (Primary)
7.2.1 **Native Swift + RealityKit + ARKit** for scan, view, splats, passthrough compositor.  
7.2.2 **`SOSCoreBridge`** → `sos_c_api.h` — all room/media state through shared core.  
7.2.3 Head pose: **ARKit / RealityKit** — automatic, latency-optimized; not Qt Sensors.

### 7.3 Optional Qt Path
7.3.1 Qt Quick 3D Xr only if shared QML with Quest is prioritized over native splat UX.  
7.3.2 Prebuilt Qt: build from **[qt-visionos-prebuilt](https://github.com/davecotter/qt-visionos-prebuilt)** (community source of truth).

### 7.4 Minimum UI
7.4.1 Start scan / stop scan.  
7.4.2 Import splat file.  
7.4.3 Save room.  
7.4.4 Open room / view room.  
7.4.5 Place screen/speaker; pick local media file.  
7.4.6 Basic alignment status indicator (aligned / provisional / lost).

## 8. Minimum Quest 3 App

### 8.1 Required Features
8.1.1 Same viewer capabilities as Vision Pro: scan, import, save, view, **local media**.  

### 8.2 Platform Stack
8.2.1 **Qt Quick 3D Xr** (recommended for shared shell) or native OpenXR.  
8.2.2 Passthrough + mesh/splat render; head pose from **OpenXR** runtime.

### 8.3 Cross-Platform SWM Compatibility
8.3.1 Quest MUST read room bundles from iOS and Vision Pro when asset version matches.  
8.3.2 GPS metadata preserved for future federation; alignment on Quest may need bootstrap scan.

## 9. Shared Media Playback (Phase 0)

### 9.1 Product Goal
9.1.1 Users place a **virtual screen** or **speaker** in the room and play **local movie/audio files** — shared watching/listening in spatial context.  
9.1.2 API MUST abstract source type so **URL streaming** (YouTube, Netflix, etc.) can be added later without rewriting objects.

### 9.2 Core Contract (`MediaSource`)
```json
{
  "source_kind": "local_file | url_stream (reserved)",
  "uri": "file://... or https://...",
  "mime_hint": "video/mp4",
  "loop": false
}
```

### 9.3 Platform Backends (Phase 0)
| Platform | Backend |
|----------|---------|
| Vision Pro | AVFoundation / RealityKit video material |
| iOS | AVFoundation (preview only if needed) |
| Quest | Qt Multimedia or native Android MediaPlayer |

### 9.4 Deferred (not Phase 0)
9.4.1 **Qt WebView** in picture frames — web login + “fullscreen to frame” for Netflix/YouTube (DRM, ToS, latency) — see Part 12 §4.4.9.  
9.4.2 Remote stream sync across users (Phase 1+).

## 10. End-to-End User Flows

### 10.1 Flow A: Scan Room on iOS → View on Vision Pro
10.1.1 User scans with iOS app → exports `.sosroom` (with GPS).  
10.1.2 User imports on Vision Pro → view aligned + optional local movie on virtual screen.

### 10.2 Flow B: Scan on Vision Pro
10.2.1 User opens app → Scan mode → Save → View mode.

### 10.3 Flow C: Import Splat on Quest 3
10.3.1 Import splat → save → view (bootstrap alignment if needed).

### 10.4 Flow D: Cross-Device Bundle
10.4.1 Export from any contributor → import on another platform.

### 10.5 Flow E: Shared Local Movie
10.5.1 User places virtual screen → selects local video file → playback visible in room.

## 11. Acceptance Criteria

### 11.1 Core Abstraction
11.1.1 One shared import abstraction used by **iOS, Vision Pro, Quest**.  
11.1.2 Sensor and splat paths produce valid `RoomRecord` + assets.  
11.1.3 `MediaSource` local_file works on Vision Pro and Quest.

### 11.2 iOS App
11.2.1 LiDAR scan → export bundle completes.  
11.2.2 GPS written when location permission granted.

### 11.3 Vision Pro App
11.3.1 Scan/save/view loop; splat import; local video on virtual screen.

### 11.4 Quest 3 App
11.4.1 Same viewer loop; reads bundles from iOS/Vision Pro (best-effort alignment).

### 11.5 Performance
11.5.1 Interactive frame rate in view mode; room load under 3s typical.

## 12. Deferred to Phase 1+

### 12.1 Phase 1 Candidates
12.1.1 **Generic Android** (non-Quest) client.  
12.1.2 **Qt Positioning** / geospatial room index.  
12.1.3 Local mesh sync (BLE/mDNS).  
12.1.4 Multi-room graph.

### 12.2 Phase 2+ Candidates
12.2.1 **Web picture frames** — Qt WebView, streaming service login, DRM (Part 12 §4.4.9).  
12.2.2 WAN federation and remote avatar viewing.  
12.2.3 Virtual object ownership handoff (Part 9).  
12.2.4 AI agent accounts (Cursor, Claude, GPT, Gemini) + MCP manipulation.
