# SOS Spec Part 4: Platforms, MCP, and Delivery Plan

## Table of Contents

- [1. Platform Support Matrix](#1-platform-support-matrix)
- [2. Adapter Implementation Strategy](#2-adapter-implementation-strategy)
- [3. Scaniverse Integration](#3-scaniverse-integration)
- [4. MCP Interface Specification](#4-mcp-interface-specification)
- [5. Node Appliance Packaging](#5-node-appliance-packaging)
- [6. Security and Governance Delivery Requirements](#6-security-and-governance-delivery-requirements)
- [7. MVP to GA Phase Plan](#7-mvp-to-ga-phase-plan)
- [8. Test and Certification Plan](#8-test-and-certification-plan)
- [9. Implementation Backlog Seed](#9-implementation-backlog-seed)

## 1. Platform Support Matrix

### 1.1 Tier 1 (MVP Critical)
1.1.1 iOS/iPadOS (mobile AR capture + render).  
1.1.2 Android (mobile AR capture + render).  
1.1.3 Apple Vision Pro (high-fidelity immersive runtime).  
1.1.4 Meta Quest family (OpenXR immersive runtime).  
1.1.5 Desktop preview/runtime (developer and analyst workflows).

### 1.2 Tier 2 (Post-MVP)
1.2.1 SteamVR class devices.  
1.2.2 PS VR pathway through approved platform adapter model.  
1.2.3 XR glasses families through OpenXR/native adapter combinations.

## 2. Adapter Implementation Strategy

### 2.1 Adapter Contract Areas
2.1.1 Sensor ingest.  
2.1.2 Tracking/pose.  
2.1.3 Reconstruction output bridge.  
2.1.4 Render backend.  
2.1.5 Input/action mapping.

### 2.2 Adapter Maturity Levels
2.2.1 Experimental: limited confidence, no safety-critical modes.  
2.2.2 Beta: full sync + analyst mode.  
2.2.3 Production: replacement mode allowed with certified safety profile.

## 3. Scaniverse Integration

### 3.1 Input
3.1.1 Import Scaniverse capture outputs into SWM asset model.  
3.1.2 Normalize coordinate frames and annotate provenance.

### 3.2 Rendering
3.2.1 Render Scaniverse-origin assets with LOD and streaming constraints.  
3.2.2 Preserve interaction parity with native SOS capture outputs.

### 3.3 Platform Reference
3.3.1 [Scaniverse](https://dev.scaniverse.com/)

## 4. MCP Interface Specification

### 4.1 Core Query Tools
4.1.1 `get_world_state`  
4.1.2 `get_room_graph`  
4.1.3 `query_entities`  
4.1.4 `get_confidence_map`

### 4.2 Core Mutation Tools
4.2.1 `submit_observation`  
4.2.2 `propose_entity_update`  
4.2.3 `merge_room_candidates`  
4.2.4 `set_semantic_label`

### 4.3 Control and Simulation Tools
4.3.1 `simulate_traversal`  
4.3.2 `recompute_alignment`  
4.3.3 `run_confidence_audit`

### 4.4 Tooling Rules
4.4.1 MCP mutations must pass authorization and safety policy checks.  
4.4.2 All mutation calls emit auditable event records.

## 5. Node Appliance Packaging

### 5.1 Packaging Targets
5.1.1 Container image.  
5.1.2 VM image.  
5.1.3 One-click local launcher bundle.

### 5.2 Bootstrap Experience
5.2.1 Start node appliance.  
5.2.2 Broadcast BLE/mDNS discovery hints.  
5.2.3 Pair local clients and join mesh with policy defaults.

## 6. Security and Governance Delivery Requirements

### 6.1 Security Baseline at MVP
6.1.1 Mutual auth and signed identity claims.  
6.1.2 Encryption in transit and at rest.  
6.1.3 Permission scopes for room/node/entity mutation.

### 6.2 Governance Controls
6.2.1 Trusted operator roles.  
6.2.2 Escalation workflows for disputed merges.  
6.2.3 Moderation actions and rollback procedures.

## 7. MVP to GA Phase Plan

### 7.0 Phase 0 (First Goal — Local Room)
7.0.1 Platform-agnostic import abstraction (sensor live + splat file).  
7.0.2 Local SWM read/write embedded in app.  
7.0.3 Minimum Vision Pro app: scan, import, save, view aligned over real room.  
7.0.4 Minimum Quest 3 app: same capabilities on shared core.  
7.0.5 See `spec-10-mvp-phase-0-local-room.md` for full acceptance criteria.

### 7.1 Phase A (Foundations)
7.1.1 SWM core schema and event model (started in Phase 0 local store).  
7.1.2 Single-node appliance packaging (after embedded local SWM proves out).  
7.1.3 Mobile capture + desktop preview.

### 7.2 Phase B (Immersive Clients)
7.2.1 Vision Pro and Quest immersive render adapters (started in Phase 0).  
7.2.2 Ghost room lifecycle and traversal reconciliation.

### 7.3 Phase C (Federation and Scale)
7.3.1 Inter-node mesh and radius policy enforcement.  
7.3.2 Cross-boundary traversal handoff.

### 7.4 Phase D (Hardening)
7.4.1 Safety certification gates.  
7.4.2 Conformance suite and performance SLO validation.

## 8. Test and Certification Plan

### 8.1 Required Suites
8.1.1 Schema validation and migration tests.  
8.1.2 CRDT convergence and partition simulation tests.  
8.1.3 Cross-platform render equivalence tests.  
8.1.4 Safety mode downgrade and fail-safe tests.

### 8.2 Certification Gates
8.2.1 Adapter conformance.  
8.2.2 Security baseline conformance.  
8.2.3 Operational SLO conformance.

## 9. Implementation Backlog Seed

### 9.1 Core Workstreams
9.1.1 SWM schema package and validator.  
9.1.2 Event bus and CRDT engine.  
9.1.3 Node appliance runtime.  
9.1.4 Discovery subsystem (BLE + mDNS + WAN).  
9.1.5 Mobile capture clients.  
9.1.6 Vision Pro and Quest adapters.  
9.1.7 Desktop analyst client.  
9.1.8 MCP server and tooling.

### 9.2 Initial Milestone Definition
9.2.1 **Phase 0 milestone**: scan or import splat → save local SWM → read local SWM → render over real room on Vision Pro and Quest 3.  
9.2.2 **Phase 1 milestone**: "First linked house map": two rooms, one doorway, ghost-room alignment, cross-device live sync.
