# SOS Spec Part 5: Sensing and Reconstruction Implementation

## Table of Contents

- [1. Purpose and Normative Scope](#1-purpose-and-normative-scope)
- [2. Shared Capture Pipeline Contract](#2-shared-capture-pipeline-contract)
- [3. iOS/iPadOS Capture Implementation](#3-iosipados-capture-implementation)
- [4. visionOS (Vision Pro) Capture Implementation](#4-visionos-vision-pro-capture-implementation)
- [5. Quest Capture Implementation](#5-quest-capture-implementation)
- [6. Scaniverse Import Pipeline](#6-scaniverse-import-pipeline)
- [7. Reconstruction Backends](#7-reconstruction-backends)
- [8. Semantic and Affordance Inference](#8-semantic-and-affordance-inference)
- [9. Observation Packaging and Upload](#9-observation-packaging-and-upload)
- [10. Validation, Metrics, and Failure Handling](#10-validation-metrics-and-failure-handling)

## 1. Purpose and Normative Scope

### 1.1 Purpose
1.1.1 Define exactly how capture and reconstruction are implemented across device families and import sources.  
1.1.2 Remove ambiguity from generic statements like "sensor frame capture" by defining per-platform behavior.

### 1.2 Normative Terms
1.2.1 MUST indicates mandatory behavior.  
1.2.2 SHOULD indicates recommended behavior unless a justified exception exists.  
1.2.3 MAY indicates optional behavior.

## 2. Shared Capture Pipeline Contract

### 2.1 Canonical Capture Stages
2.1.1 Frame acquisition.  
2.1.2 Pose estimation update.  
2.1.3 Depth/geometry extraction.  
2.1.4 Local reconstruction update.  
2.1.5 Semantic/affordance tagging.  
2.1.6 Observation packaging.  
2.1.7 Sync submission.

### 2.2 Canonical In-Memory Frame Type
2.2.1 Each platform adapter MUST emit `CaptureFrame`:
```json
{
  "frame_id": "uuid",
  "timestamp_ns": 0,
  "platform": "ios|visionos|quest|scaniverse_import",
  "camera": {
    "intrinsics": [0,0,0,0,0,0,0,0,0],
    "resolution": [0,0],
    "exposure_ms": 0.0
  },
  "pose_world_from_device": [0,0,0,0,0,0,0],
  "depth": {
    "available": false,
    "format": "none|depth16|float32",
    "resolution": [0,0]
  },
  "imu": {
    "gyro_rad_s": [0,0,0],
    "accel_m_s2": [0,0,0]
  },
  "confidence": {
    "pose": 0.0,
    "depth": 0.0,
    "tracking": 0.0
  }
}
```

### 2.3 Processing Threads
2.3.1 Capture thread MUST never block on network I/O.  
2.3.2 Reconstruction thread SHOULD process bounded batches (for example 10-30 frames).  
2.3.3 Upload thread MUST be backpressure-aware and spool to local queue under congestion.

### 2.4 Time Synchronization
2.4.1 All frames MUST be timestamped in monotonic nanoseconds.  
2.4.2 Session startup MUST record wall-clock/monotonic offset for cross-device reconciliation.

## 3. iOS/iPadOS Capture Implementation

### 3.1 Session Setup
3.1.1 Adapter initializes AR session with world tracking.  
3.1.2 If LiDAR is available, adapter MUST enable scene depth path.  
3.1.3 If LiDAR unavailable, adapter MUST enable monocular depth estimation fallback.

### 3.2 Per-Frame Flow
3.2.1 Acquire camera frame and tracking state.  
3.2.2 Extract camera intrinsics and transform matrix.  
3.2.3 Convert platform transform into SOS canonical quaternion+translation representation.  
3.2.4 Attach depth map (LiDAR if present, estimated depth if fallback mode).

### 3.3 Tracking State Mapping
3.3.1 Platform tracking quality MUST map to normalized `tracking_confidence`:
- `normal` -> 0.85 to 1.00
- `limited` -> 0.40 to 0.84
- `unavailable` -> 0.00 to 0.39

### 3.4 iOS-Specific Failure Handling
3.4.1 If tracking transitions to unavailable for > 2.0s, capture MUST pause mutation uploads and mark observations as provisional.  
3.4.2 If thermal state reaches critical, adapter SHOULD reduce frame processing cadence before dropping session.

### 3.5 Output Contract
3.5.1 iOS adapter MUST output:
- sparse point observations every frame
- dense geometry updates at throttled interval (for example 2-5 Hz)
- semantic hints where platform APIs permit

## 4. visionOS (Vision Pro) Capture Implementation

### 4.1 Session Setup
4.1.1 Adapter MUST initialize spatial tracking using platform-native APIs optimized for headset motion model.  
4.1.2 Eye/hand/head streams MAY be attached as interaction metadata, but MUST be separated from geometry capture payloads.

### 4.2 Pose and Space Management
4.2.1 Adapter MUST bind to a stable world reference space for room capture sessions.  
4.2.2 If reference-space relocalization occurs, adapter MUST emit a relocalization event with old/new transform mapping.

### 4.3 Depth and Reconstruction Inputs
4.3.1 If depth channel is available, adapter MUST provide metric-depth maps to reconstruction backend.  
4.3.2 If depth channel is unavailable for a frame, adapter SHOULD infer geometric updates using temporal consistency and prior model constraints.

### 4.4 Vision Pro Quality Policy
4.4.1 For full replacement runtime eligibility, rolling pose drift metric MUST stay under configured threshold (for example < 2 cm over 10 s local loop).  
4.4.2 If threshold exceeded, runtime MUST downgrade to mixed mode until recertified.

## 5. Quest Capture Implementation

### 5.1 Session Setup
5.1.1 Adapter MUST initialize OpenXR session and query supported extensions for passthrough/depth/scene understanding.  
5.1.2 Unsupported features MUST be explicitly disabled via capability descriptor.

### 5.2 Frame Acquisition
5.2.1 Pose sampling MUST use predicted display time aligned with OpenXR frame loop.  
5.2.2 Camera/depth ingestion SHOULD use vendor-provided APIs when available, otherwise fallback to tracked-geometry-only observation mode.

### 5.3 Coordinate Conversion
5.3.1 Adapter MUST convert OpenXR reference space transforms into SOS canonical world coordinates.  
5.3.2 Handedness and axis conversions MUST be centralized in one tested transform utility.

### 5.4 Quest-Specific Constraints
5.4.1 If raw camera access is restricted by platform policy, adapter MUST continue with inferred map updates and mark `sensor_mode = inferred`.  
5.4.2 In inferred mode, semantic confidence MUST be capped below direct-observation ceiling.

## 6. Scaniverse Import Pipeline

### 6.1 Ingest Modes
6.1.1 File import mode (local package).  
6.1.2 URL import mode (remote asset reference).  
6.1.3 Streaming ingest mode (progressive asset chunks).

### 6.2 Required Imported Metadata
6.2.1 Source platform id (`scaniverse`).  
6.2.2 Capture timestamp and location (if available).  
6.2.3 Native coordinate frame declaration.  
6.2.4 Asset modality (`mesh`, `point_cloud`, `gaussian_splat`).

### 6.3 Coordinate Normalization
6.3.1 Import adapter MUST compute `T_world_from_source` before ingesting geometry.  
6.3.2 If absolute georeference is unavailable, import MUST start in provisional room state and require alignment events.

### 6.4 Gaussian Splat Import
6.4.1 Adapter MUST parse splat parameters into canonical `SplatAsset` structure:
- center position
- covariance/scale basis
- opacity
- color
- optional normal/confidence channels
6.4.2 Renderer compatibility layer MUST generate fallback mesh proxy for clients without splat rendering support.

### 6.5 Provenance and Confidence
6.5.1 Imported assets MUST include immutable provenance tags.  
6.5.2 Initial confidence MUST be bootstrapped from import quality heuristics and later refined by live observations.

## 7. Reconstruction Backends

### 7.1 Backend Types
7.1.1 Incremental TSDF/voxel fusion backend.  
7.1.2 Sparse-to-dense point cloud backend.  
7.1.3 Gaussian splat reconstruction backend.

### 7.2 Backend Selection Rules
7.2.1 Mobile low-power profile SHOULD default to sparse+dense hybrid.  
7.2.2 High-end headset profile MAY enable full splat reconstruction path.  
7.2.3 Import-only sessions MAY skip live reconstruction and use alignment-only pipeline.

### 7.3 Reconstruction Output Contract
7.3.1 Backend MUST emit `ReconstructionDelta`:
```json
{
  "delta_id": "uuid",
  "room_id": "uuid",
  "artifact_refs": ["asset://..."],
  "coverage_ratio": 0.0,
  "mean_reprojection_error_px": 0.0,
  "geometry_confidence": 0.0,
  "bbox_world": [0,0,0,0,0,0]
}
```

## 8. Semantic and Affordance Inference

### 8.1 Semantic Labeling Flow
8.1.1 Candidate labels produced from vision model and geometric heuristics.  
8.1.2 Labels merged with prior labels using confidence-aware reconciliation.

### 8.2 Affordance Derivation
8.2.1 Affordances MUST be derived from semantic class + geometry + support constraints.  
8.2.2 Example: a bed-like horizontal support surface may emit `lie_down`, `sit`, `kneel` with confidence tiers.

### 8.3 Human Correction
8.3.1 Manual user relabeling MUST be represented as explicit events with actor provenance.  
8.3.2 Manual corrections SHOULD receive elevated trust but remain reversible.

## 9. Observation Packaging and Upload

### 9.1 Observation Record
9.1.1 Each upload unit MUST include:
- source session id
- device class and adapter version
- local pose transform
- confidence vector
- reconstruction delta refs
- semantic delta refs
- policy flags

### 9.2 Upload Policies
9.2.1 Critical topology updates (doorway/link changes) MUST use acked channel.  
9.2.2 High-rate telemetry MAY use best-effort channel.

### 9.3 Batching
9.3.1 Batching window SHOULD be adaptive:
- low latency mode: 100-250 ms
- balanced mode: 250-750 ms
- constrained network mode: 1-3 s

## 10. Validation, Metrics, and Failure Handling

### 10.1 Validation Rules
10.1.1 Reject frames with invalid intrinsics or non-finite transforms.  
10.1.2 Reject reconstruction deltas with impossible bounding boxes.

### 10.2 Metrics
10.2.1 Required metrics:
- frame ingest rate
- dropped-frame ratio
- reconstruction latency
- reprojection error
- confidence trend by source

### 10.3 Failure Handling
10.3.1 On sustained capture failure, runtime MUST switch to render-only subscriber mode when possible.  
10.3.2 On upload failure, runtime MUST persist local op queue and retry with exponential backoff.
