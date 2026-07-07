# Spatial Operating System (SOS) Comprehensive Specification

## Table of Contents

- [0. First Goal — MVP Phase 0 (Local Room)](#0-first-goal--mvp-phase-0-local-room)
- [1. Product Introduction](#1-product-introduction)
- [2. Product Architecture Overview](#2-product-architecture-overview)
- [3. Core Implementation Requirements](#3-core-implementation-requirements)
- [4. Implementation Document Set](#4-implementation-document-set)
- [5. Delivery Gates](#5-delivery-gates)

## 0. First Goal — MVP Phase 0 (Local Room)

### 0.1 Objective
0.1.1 Build a **platform-agnostic import abstraction** for room models from **live sensors** or **existing Gaussian splats**, then ship **minimum iOS (capture), Vision Pro, and Quest 3** apps that **write/read local SWM**, **render over the real-world room**, and play **local shared media**.

### 0.2 Build Sequence
0.2.1 [Phase 0 implementation plan — toolchains, libraries, repo layout](./spec-12-phase-0-implementation-plan.md).  
0.2.2 [Shared import abstraction + local SWM store](./spec-10-mvp-phase-0-local-room.md#4-platform-agnostic-import-abstraction).  
0.2.3 [Minimum iOS app (LiDAR capture + GPS export)](./spec-10-mvp-phase-0-local-room.md#6-minimum-ios-app-lidar-capture).  
0.2.4 [Minimum Vision Pro app](./spec-10-mvp-phase-0-local-room.md#7-minimum-vision-pro-app).  
0.2.5 [Minimum Quest 3 app](./spec-10-mvp-phase-0-local-room.md#8-minimum-quest-3-app).  
0.2.6 [Shared media playback (Phase 0)](./spec-10-mvp-phase-0-local-room.md#9-shared-media-playback-phase-0).  
0.2.7 [Splat file import path](./spec-10-mvp-phase-0-local-room.md#35-step-5--splat-import-path).

### 0.3 Phase 0 App Capabilities (All Three Platforms)
0.3.1 Import/scan with sensors → normalize → save to local SWM (iOS export; VP/Quest view).  
0.3.2 Import existing splat file → normalize → save to local SWM.  
0.3.3 Read from local SWM on launch.  
0.3.4 Render local node aligned over physical room (high immersion on Vision Pro / Quest).  
0.3.5 Local video/audio on virtual screens/speakers (`MediaSource` abstraction).

### 0.4 Phase 0 Non-Goals
0.4.1 Federation, remote mesh, virtual object ownership, multi-room ghost graph — deferred. See [Phase 0 scope](./spec-10-mvp-phase-0-local-room.md#2-scope-in-and-out).

## 1. Product Introduction

### 1.1 Product Definition
1.1.1 SOS is a cross-platform spatial operating system that senses, models, synchronizes, and renders physical environments as a shared world model.  
1.1.2 The product supports AR overlay, mixed mode, and immersive replacement while preserving real-world alignment, affordances, and safety behavior.

### 1.2 Primary User Outcomes
1.2.1 Users can scan rooms and environments from supported devices.  
1.2.2 Users can enter the same shared space from multiple hardware platforms.  
1.2.3 Users can view and interact with a continuously improving model of reality.  
1.2.4 Authorized users and devices can contribute higher-quality observations that improve shared truth over time.  
1.2.5 Users can create or import ownable virtual objects (picture frames, speakers, video screens), transfer ownership by handoff, and place objects in spaces under permission rules.  
1.2.6 Property owners can hold **notarized Property Model Rights (PMR)** for spatial models bound to real estate parcels; PMR transfers with property sale, separate from portable virtual objects.

### 1.3 Product Runtime Model
1.3.1 One application supports multiple runtime roles: `capture`, `render`, `hybrid`, and `analyst`.  
1.3.2 Nodes can run locally or remotely as appliance deployments and federate into a geospatial mesh.

## 2. Product Architecture Overview

### 2.1 Architecture Layers
2.1.1 [Capture and sensing layer](./spec-05-sensing-and-reconstruction-implementation.md#2-shared-capture-pipeline-contract).  
2.1.2 [Spatial World Model (SWM) and reconciliation layer](./spec-03-data-model-protocols-and-federation.md#1-swm-schema-baseline).  
2.1.3 [Sync and federation layer (event log + CRDT + routing)](./spec-03-data-model-protocols-and-federation.md#5-crdt-strategy).  
2.1.4 [Rendering and interaction layer](./spec-07-rendering-safety-and-interaction-implementation.md#1-rendering-architecture-contract).  
2.1.5 [Platform adapter layer](./spec-04-platforms-mcp-and-delivery-plan.md#2-adapter-implementation-strategy).  
2.1.6 [MCP and operational control layer](./spec-04-platforms-mcp-and-delivery-plan.md#4-mcp-interface-specification).

### 2.2 Core Product Capabilities
2.2.1 [Shared world model with confidence and provenance](./spec-03-data-model-protocols-and-federation.md#3-confidence-model).  
2.2.2 [Room/doorway topology with ghost-room reconciliation](./spec-02-reference-architecture-and-runtime.md#7-ghost-room-lifecycle).  
2.2.3 [Cross-platform runtime through adapter contracts](./spec-04-platforms-mcp-and-delivery-plan.md#2-adapter-implementation-strategy).  
2.2.4 [Local BLE + mDNS discovery and WAN federation](./spec-03-data-model-protocols-and-federation.md#7-discovery-protocols).  
2.2.5 [Safety-gated immersive replacement modes](./spec-02-reference-architecture-and-runtime.md#8-safety-and-policy-runtime).  
2.2.6 [Spatial URL addressing and cross-node traversal continuity](./spec-03-data-model-protocols-and-federation.md#8-spatial-url-and-addressing).  
2.2.7 [Ownable virtual objects with handoff, placement permissions, and owner-controlled content](./spec-09-virtual-objects-ownership-and-avatars.md#3-ownership-and-permission-model).  
2.2.8 [Property Model Rights: real estate–bound models, notarization, transfer on sale](./spec-11-property-model-rights-and-notarization.md#2-two-ownership-layers).

### 2.3 Platform Strategy
2.3.1 [Phase 0 first ship targets: Vision Pro + Quest 3 minimum apps](./spec-10-mvp-phase-0-local-room.md#1-first-goal-statement).  
2.3.2 [Phase 1 expansion: iOS/Android mobile AR, desktop preview](./spec-04-platforms-mcp-and-delivery-plan.md#11-tier-1-mvp-critical).  
2.3.3 [Later: SteamVR, PS VR, XR glasses](./spec-04-platforms-mcp-and-delivery-plan.md#12-tier-2-post-mvp).  
2.3.4 [Scaniverse/splat import supported via shared import abstraction](./spec-10-mvp-phase-0-local-room.md#42-import-source-types).

## 3. Core Implementation Requirements

### 3.1 Data and Model Requirements
3.1.1 [SWM is the system of record for geometry, semantics, affordances, confidence, and provenance](./spec-06-swm-sync-and-protocol-implementation.md#2-swm-schema-definition-normative).  
3.1.2 [All mutations are event-based and converge through defined CRDT/conflict policies](./spec-06-swm-sync-and-protocol-implementation.md#6-crdt-implementation-rules).  
3.1.3 [OSM is a bootstrap prior only and is superseded by higher-confidence SOS observations](./spec-03-data-model-protocols-and-federation.md#11-osm-integration-rules).

### 3.2 Runtime and Safety Requirements
3.2.1 [Runtime roles are capability-driven and policy-gated](./spec-02-reference-architecture-and-runtime.md#3-runtime-roles).  
3.2.2 [Immersive replacement requires confidence and safety checks with downgrade paths](./spec-02-reference-architecture-and-runtime.md#8-safety-and-policy-runtime).  
3.2.3 [Traversal across room and node boundaries must preserve pose continuity and state consistency](./spec-03-data-model-protocols-and-federation.md#9-federation-rules).

### 3.3 Network and Federation Requirements
3.3.1 [Discovery uses BLE + mDNS locally and federation discovery remotely](./spec-06-swm-sync-and-protocol-implementation.md#8-discovery-and-federation-protocol-details).  
3.3.2 [Node connectivity follows Earth-native geodesic/radius constraints](./spec-03-data-model-protocols-and-federation.md#10-geospatial-routing-and-radius-policies).  
3.3.3 [Federation replication and routing obey trust, proximity, and policy controls](./spec-06-swm-sync-and-protocol-implementation.md#8-discovery-and-federation-protocol-details).

### 3.4 Control Plane Requirements
3.4.1 [MCP is the standard operational interface for query, mutation, simulation, and audits](./spec-04-platforms-mcp-and-delivery-plan.md#4-mcp-interface-specification).  
3.4.2 [Mutating operations require authorization and emit auditable records](./spec-04-platforms-mcp-and-delivery-plan.md#44-tooling-rules).

### 3.5 Property Model Rights and Virtual Objects
3.5.1 [PMR: property owner owns the model of their house/parcel, not guests' virtual objects](./spec-11-property-model-rights-and-notarization.md#3-property-model-rights-pmr).  
3.5.2 [Notarization and attestation for legally meaningful ownership binding](./spec-11-property-model-rights-and-notarization.md#5-notarization-and-attestation).  
3.5.3 [PMR transfers to buyer on real estate sale; virtual objects do not](./spec-11-property-model-rights-and-notarization.md#9-property-transfer-on-real-estate-sale).  
3.5.4 [Public streets/spaces: public authority owns PMR; creators credited but do not own](./spec-11-property-model-rights-and-notarization.md#7-public-vs-private-property).  
3.5.5 [Portable virtual objects: handoff, placement, content ownership (Part 9)](./spec-09-virtual-objects-ownership-and-avatars.md#4-handoff-and-transfer-protocol).

### 3.6 Platform-Specific Capture Requirements
3.6.1 [iOS/iPadOS capture flow](./spec-05-sensing-and-reconstruction-implementation.md#3-iosipados-capture-implementation).  
3.6.2 [Vision Pro capture flow](./spec-05-sensing-and-reconstruction-implementation.md#4-visionos-vision-pro-capture-implementation).  
3.6.3 [Quest capture flow](./spec-05-sensing-and-reconstruction-implementation.md#5-quest-capture-implementation).  
3.6.4 [Scaniverse import flow](./spec-05-sensing-and-reconstruction-implementation.md#6-scaniverse-import-pipeline).

## 4. Implementation Document Set

### 4.1 Part 1: Product Decisions
4.1.1 File: `spec-01-prompt-responses-and-product-decisions.md`  
4.1.2 Contains complete architecture-grade answers for each product requirement prompt.

### 4.2 Part 2: Runtime Architecture
4.2.1 File: `spec-02-reference-architecture-and-runtime.md`  
4.2.2 Defines topology, pipelines, runtime modes, safety runtime, and failure recovery.

### 4.3 Part 3: Data, Protocols, Federation
4.3.1 File: `spec-03-data-model-protocols-and-federation.md`  
4.3.2 Defines SWM schema baseline, confidence model, event/CRDT contracts, protocol and routing rules.

### 4.4 Part 4: Platforms and Delivery
4.4.1 File: `spec-04-platforms-mcp-and-delivery-plan.md`  
4.4.2 Defines platform adapter scope, MCP surfaces, appliance packaging, and phased delivery.

### 4.5 Part 5: Sensing and Reconstruction Implementation
4.5.1 File: `spec-05-sensing-and-reconstruction-implementation.md`  
4.5.2 Defines per-platform capture pipelines, reconstruction backends, semantic inference, and observation upload details.

### 4.6 Part 6: SWM, Sync, and Protocol Implementation
4.6.1 File: `spec-06-swm-sync-and-protocol-implementation.md`  
4.6.2 Defines normative schemas, CRDT rules, wire protocol, discovery/federation payloads, and persistence/migration behavior.

### 4.7 Part 7: Rendering, Safety, and Interaction Implementation
4.7.1 File: `spec-07-rendering-safety-and-interaction-implementation.md`  
4.7.2 Defines render frame graph, runtime mode implementation, replacement safety checks, avatar/analyst cameras, and performance adaptation.

### 4.8 Part 8: Delivery, Test, and Operations Implementation
4.8.1 File: `spec-08-delivery-test-and-operations-implementation.md`  
4.8.2 Defines CI/CD, conformance execution, security operations, node appliance operations, incident recovery, and milestone delivery.

### 4.9 Part 9: Virtual Objects, Ownership, and Avatars
4.9.1 File: `spec-09-virtual-objects-ownership-and-avatars.md`  
4.9.2 Defines ownable virtual object schema, handoff ownership transfer, placement/content permissions, object type specs, and avatar import roadmap.

### 4.10 Part 10: MVP Phase 0 — Local Room
4.10.1 File: `spec-10-mvp-phase-0-local-room.md`  
4.10.2 Defines the first implementation goal: platform-agnostic import abstraction, local SWM, minimum Vision Pro and Quest 3 apps, acceptance criteria.

### 4.11 Part 11: Property Model Rights and Notarization
4.11.1 File: `spec-11-property-model-rights-and-notarization.md`  
4.11.2 Defines real estate–bound model ownership, notarization/attestation, SAN enforcement, public vs private property, and transfer on sale.

### 4.12 Part 12: Phase 0 Implementation Plan (Vision Pro + Quest 3)
4.12.1 File: `spec-12-phase-0-implementation-plan.md`  
4.12.2 Defines real toolchains, required libraries, repository layout, build commands, milestones M0–M3, and links to scaffolded code under `/Volumes/Developer/git/openworld/`.

## 5. Delivery Gates

### 5.1 Engineering Gates
5.1.1 [SWM schema and migrations implemented](./spec-06-swm-sync-and-protocol-implementation.md#2-swm-schema-definition-normative).  
5.1.2 [Event/CRDT semantics and convergence tests implemented](./spec-06-swm-sync-and-protocol-implementation.md#6-crdt-implementation-rules).  
5.1.3 [Discovery, protocol, and federation contracts implemented](./spec-06-swm-sync-and-protocol-implementation.md#7-session-and-wire-protocol).

### 5.2 Product Gates
5.2.1 [Phase 0: iOS + Vision Pro + Quest scan/import/save/view + local media](./spec-10-mvp-phase-0-local-room.md#11-acceptance-criteria).  
5.2.2 [Phase 1+: broader platform matrix and role-based runtime](./spec-04-platforms-mcp-and-delivery-plan.md#1-platform-support-matrix).  
5.2.3 [Phase 1+: ghost-room and traversal reconciliation](./spec-02-reference-architecture-and-runtime.md#7-ghost-room-lifecycle).

### 5.3 Operational Gates
5.3.1 [Node appliance deployable locally and remotely](./spec-08-delivery-test-and-operations-implementation.md#7-node-appliance-operations).  
5.3.2 [Security, governance, and audit controls operational](./spec-08-delivery-test-and-operations-implementation.md#5-security-operations).  
5.3.3 [Performance and safety SLOs validated](./spec-08-delivery-test-and-operations-implementation.md#6-observability-implementation).

---

## Specification Parts (Quick Links)

- [Part 1: Prompt Responses and Product Decisions](./spec-01-prompt-responses-and-product-decisions.md)
- [Part 2: Reference Architecture and Runtime Design](./spec-02-reference-architecture-and-runtime.md)
- [Part 3: Data Model, Protocols, and Federation](./spec-03-data-model-protocols-and-federation.md)
- [Part 4: Platforms, MCP, and Delivery Plan](./spec-04-platforms-mcp-and-delivery-plan.md)
- [Part 5: Sensing and Reconstruction Implementation](./spec-05-sensing-and-reconstruction-implementation.md)
- [Part 6: SWM, Sync, and Protocol Implementation](./spec-06-swm-sync-and-protocol-implementation.md)
- [Part 7: Rendering, Safety, and Interaction Implementation](./spec-07-rendering-safety-and-interaction-implementation.md)
- [Part 8: Delivery, Test, and Operations Implementation](./spec-08-delivery-test-and-operations-implementation.md)
- [Part 9: Virtual Objects, Ownership, and Avatars](./spec-09-virtual-objects-ownership-and-avatars.md)
- [Part 10: MVP Phase 0 — Local Room (First Goal)](./spec-10-mvp-phase-0-local-room.md)
- [Part 11: Property Model Rights and Notarization](./spec-11-property-model-rights-and-notarization.md)
- [Part 12: Phase 0 Implementation Plan — Vision Pro + Quest 3](./spec-12-phase-0-implementation-plan.md)
