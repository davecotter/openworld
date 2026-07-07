# SOS Spec Part 2: Reference Architecture and Runtime Design

## Table of Contents

- [1. Runtime Topology](#1-runtime-topology)
- [2. Core Services](#2-core-services)
- [3. Runtime Roles](#3-runtime-roles)
- [4. Capability Model](#4-capability-model)
- [5. Room Ingestion and Reconstruction Pipeline](#5-room-ingestion-and-reconstruction-pipeline)
- [6. Rendering Pipeline and Modes](#6-rendering-pipeline-and-modes)
- [7. Ghost Room Lifecycle](#7-ghost-room-lifecycle)
- [8. Safety and Policy Runtime](#8-safety-and-policy-runtime)
- [9. Performance Targets](#9-performance-targets)
- [10. Failure Modes and Recovery](#10-failure-modes-and-recovery)

## 1. Runtime Topology

### 1.1 Top-Level Components
1.1.1 Client runtime (mobile/VR/XR/desktop).  
1.1.2 Local or remote SOS Node Appliance.  
1.1.3 Federation gateways for inter-node traffic.

### 1.2 Logical Planes
1.2.1 Data plane (capture, state sync, render assets).  
1.2.2 Control plane (MCP/API orchestration and policy).  
1.2.3 Observability plane (logs, metrics, traces, audits).

## 2. Core Services

### 2.1 World Model Service
2.1.1 Stores SWM entities and relationships.  
2.1.2 Maintains schema versioning and migration state.

### 2.2 Reconciliation Service
2.2.1 Applies confidence propagation and conflict rules.  
2.2.2 Runs room/doorway alignment updates.

### 2.3 Sync Service
2.3.1 Maintains operation log and CRDT convergence.  
2.3.2 Handles snapshot and delta replication.

### 2.4 Asset Service
2.4.1 Stores meshes, splats, textures, and derived levels of detail.  
2.4.2 Supports chunked transfer with integrity checks.

### 2.5 Policy Service
2.5.1 Enforces mutation permissions, safety gates, and moderation.

## 3. Runtime Roles

### 3.1 Capture Role
3.1.1 Reads camera/depth/IMU streams.  
3.1.2 Produces observations and reconstruction artifacts.

### 3.2 Render Role
3.2.1 Subscribes to SWM state and asset streams.  
3.2.2 Executes AR overlay, mixed, or replacement rendering.

### 3.3 Hybrid Role
3.3.1 Combines capture and render in one process for capable devices.

### 3.4 Analyst Role
3.4.1 Enables diagnostics, confidence visualization, and scale-independent inspection.

## 4. Capability Model

### 4.1 Capability Descriptor
4.1.1 Each runtime declares capabilities at session start:
- tracking quality
- depth availability
- LiDAR availability
- stereo rendering support
- passthrough support
- maximum map extent and memory class

### 4.2 Feature Gating
4.2.1 Features requiring unsupported capabilities are automatically disabled with explicit user feedback.

## 5. Room Ingestion and Reconstruction Pipeline

### 5.1 Ingestion Stages
5.1.1 Sensor frame capture.  
5.1.2 Pose estimation and local map update.  
5.1.3 Reconstruction artifact generation (mesh/splat/point cloud).  
5.1.4 Semantic labeling and affordance inference.  
5.1.5 Observation packaging and sync submission.

### 5.2 Observation Contract
5.2.1 Required fields:
- room candidate id
- source device id/class
- local pose transform
- confidence vector
- timestamp
- artifact references

## 6. Rendering Pipeline and Modes

### 6.1 Render Inputs
6.1.1 Current user pose and motion state.  
6.1.2 SWM scene graph subset for frustum and proximity.  
6.1.3 Active policy constraints and safety overlays.

### 6.2 Runtime Modes
6.2.1 AR overlay: physical scene visible plus synthetic augmentations.  
6.2.2 Mixed mode: selective replacement regions.  
6.2.3 Full replacement: physical scene hidden; SWM-aligned synthetic representation.

### 6.3 Avatar Camera Modes
6.3.1 First-person.  
6.3.2 Over-the-shoulder.  
6.3.3 Detached observer.

## 7. Ghost Room Lifecycle

### 7.1 States
7.1.1 `ghost` (provisional geometry/location).  
7.1.2 `anchored` (doorway and traversal constraints present).  
7.1.3 `aligned` (pose graph optimized against known neighbors).  
7.1.4 `verified` (confidence threshold reached).

### 7.2 Transition Triggers
7.2.1 Sensor confidence accumulation.  
7.2.2 Traversal event through known doorway.  
7.2.3 Multi-device confirmation and outlier suppression.

## 8. Safety and Policy Runtime

### 8.1 Safety Gate Classes
8.1.1 Collision and occlusion sanity.  
8.1.2 Traversal-risk detection in replacement mode.  
8.1.3 Confidence floor checks before immersive state changes.

### 8.2 Policy Actions
8.2.1 Allow.  
8.2.2 Warn and continue.  
8.2.3 Downgrade mode (replacement -> mixed -> overlay).  
8.2.4 Block mutation and request manual confirmation.

## 9. Performance Targets

### 9.1 Interactive Targets
9.1.1 Pose-to-photon latency budget per platform profile.  
9.1.2 Continuous sync under bounded bandwidth.  
9.1.3 Deterministic convergence under concurrent edits.

### 9.2 Scalability Targets
9.2.1 Room-scale updates near real time.  
9.2.2 Building/campus updates in bounded seconds/minutes.  
9.2.3 Cross-region sync using eventual consistency and priority classes.

## 10. Failure Modes and Recovery

### 10.1 Network Partition
10.1.1 Continue local operation with queued operation log.  
10.1.2 Reconcile on reconnect with CRDT merge rules.

### 10.2 Sensor Degradation
10.2.1 Decrease confidence, limit high-risk rendering modes, and request recapture.

### 10.3 Divergent Model States
10.3.1 Trigger reconciliation jobs and analyst workflow for unresolved conflicts.

## 11. Detailed Implementation References

### 11.1 Platform Capture and Reconstruction
11.1.1 iOS implementation details: `spec-05-sensing-and-reconstruction-implementation.md#3-iosipados-capture-implementation`  
11.1.2 Vision Pro implementation details: `spec-05-sensing-and-reconstruction-implementation.md#4-visionos-vision-pro-capture-implementation`  
11.1.3 Quest implementation details: `spec-05-sensing-and-reconstruction-implementation.md#5-quest-capture-implementation`  
11.1.4 Scaniverse import details: `spec-05-sensing-and-reconstruction-implementation.md#6-scaniverse-import-pipeline`
