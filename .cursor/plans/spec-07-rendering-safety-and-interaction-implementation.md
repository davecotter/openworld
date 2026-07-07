# SOS Spec Part 7: Rendering, Safety, and Interaction Implementation

## Table of Contents

- [1. Rendering Architecture Contract](#1-rendering-architecture-contract)
- [2. Runtime Mode Implementation](#2-runtime-mode-implementation)
- [3. Full Replacement Safety Pipeline](#3-full-replacement-safety-pipeline)
- [4. Cross-Platform Render Backends](#4-cross-platform-render-backends)
- [5. Splat/Mesh Interoperability](#5-splatmesh-interoperability)
- [6. Avatar, Observer, and Analyst Cameras](#6-avatar-observer-and-analyst-cameras)
- [7. Interaction and Affordance Runtime](#7-interaction-and-affordance-runtime)
- [8. Performance Budgets and Adaptation](#8-performance-budgets-and-adaptation)
- [9. Rendering Failure Handling](#9-rendering-failure-handling)

## 1. Rendering Architecture Contract

### 1.1 Inputs
1.1.1 User pose stream.  
1.1.2 SWM scene graph subset.  
1.1.3 Asset stream (mesh/splat/material/texture).  
1.1.4 Safety policy state.  
1.1.5 Mode state (`overlay`, `mixed`, `replacement`).

### 1.2 Render Frame Graph Stages
1.2.1 Visibility culling by frustum + proximity + confidence gates.  
1.2.2 Geometry/splat selection by LOD and device profile.  
1.2.3 Occlusion and depth composition pass.  
1.2.4 Lighting/material pass.  
1.2.5 Safety overlay/debug pass.  
1.2.6 Present pass.

## 2. Runtime Mode Implementation

### 2.1 Overlay Mode
2.1.1 Physical world remains primary visual substrate.  
2.1.2 Synthetic objects composited with conservative occlusion policy.

### 2.2 Mixed Mode
2.2.1 Region-based replacement map determines where synthetic content dominates.  
2.2.2 Transition boundaries MUST be feathered and motion-stable to avoid comfort issues.

### 2.3 Replacement Mode
2.3.1 Physical view is fully replaced with SWM-aligned synthetic render.  
2.3.2 Replacement MUST be blocked or downgraded when safety conditions are not met.

### 2.4 Mode Transition Policy
2.4.1 Transition up (overlay -> mixed -> replacement) requires confidence floors and stable tracking windows.  
2.4.2 Transition down can occur immediately on safety triggers.

## 3. Full Replacement Safety Pipeline

### 3.1 Pre-Entry Checks
3.1.1 Tracking stability over rolling window.  
3.1.2 Pose drift estimate below threshold.  
3.1.3 Traversal hazard score below threshold.  
3.1.4 Critical geometry confidence above threshold.

### 3.2 Continuous Runtime Checks
3.2.1 Head velocity and predicted collision corridor check.  
3.2.2 Boundary confidence degradation check.  
3.2.3 Dynamic obstacle uncertainty check.

### 3.3 Safety Actions
3.3.1 Warning-only overlay for low-risk issues.  
3.3.2 Auto downgrade to mixed mode for medium risk.  
3.3.3 Emergency downgrade to overlay for high risk.

## 4. Cross-Platform Render Backends

### 4.1 Vision Pro Backend
4.1.1 Uses platform-native renderer bridge with spatial anchoring and low-latency pose updates.

### 4.2 Quest/OpenXR Backend
4.2.1 Uses OpenXR frame pacing and swapchain flow; extension features enabled by capability flags.

### 4.3 Mobile Backend
4.3.1 Mobile AR backend prioritizes power-aware rendering and conservative replacement gating.

### 4.4 Desktop Backend
4.4.1 Desktop backend provides analyst and preview views with extended diagnostics overlays.

## 5. Splat/Mesh Interoperability

### 5.1 Asset Selection Policy
5.1.1 Prefer splat rendering when supported and within device budget.  
5.1.2 Fallback to mesh proxy when splat path unavailable.

### 5.2 LOD Policy
5.2.1 LOD selected by distance, angular size, and frame budget.  
5.2.2 LOD transitions MUST avoid visible popping via hysteresis.

### 5.3 Material Consistency
5.3.1 Shared material parameter model ensures visual parity across mesh and splat representations.

## 6. Avatar, Observer, and Analyst Cameras

### 6.1 First-Person Camera
6.1.1 Camera rigidly follows head pose with comfort smoothing constraints.

### 6.2 Over-the-Shoulder Camera
6.2.1 Camera offset anchored to avatar body frame; collision avoidance for camera clipping is mandatory.

### 6.3 Observer Camera
6.3.1 Detached camera supports free-fly navigation with route-aware clipping and speed scaling.

### 6.4 Analyst Camera
6.4.1 Analyst mode enables scale-space zoom, confidence heatmaps, and topology overlays.

## 7. Interaction and Affordance Runtime

### 7.1 Affordance Evaluation
7.1.1 Runtime resolves candidate affordances from semantic class + local geometry + safety state.

### 7.2 Interaction Validation
7.2.1 Interactions requiring physical correspondence (e.g., kneel on bed) MUST verify local alignment confidence before activation.

### 7.3 Conflict Handling
7.3.1 If affordance confidence drops mid-interaction, runtime SHOULD degrade interaction gracefully and notify user.

## 8. Performance Budgets and Adaptation

### 8.1 Frame Budget Categories
8.1.1 Geometry/splat decode budget.  
8.1.2 Draw budget.  
8.1.3 Post-processing budget.  
8.1.4 Safety analysis budget.

### 8.2 Adaptive Degradation
8.2.1 Reduce LOD before reducing safety checks.  
8.2.2 Drop optional visual effects before reducing interaction fidelity.  
8.2.3 Never disable core safety pipeline for performance recovery.

## 9. Rendering Failure Handling

### 9.1 Render Backend Failure
9.1.1 If backend crashes or stalls, runtime MUST preserve session state and restart renderer in safe mode.

### 9.2 Pose Desync
9.2.1 If pose/render desync exceeds threshold, force downgrade to overlay and request relocalization.

### 9.3 Asset Decode Failure
9.3.1 Missing/corrupt assets MUST fallback to proxy geometry and emit telemetry/audit events.
