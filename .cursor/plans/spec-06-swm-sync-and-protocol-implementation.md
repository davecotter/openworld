# SOS Spec Part 6: SWM, Sync, and Protocol Implementation

## Table of Contents

- [1. Purpose](#1-purpose)
- [2. SWM Schema Definition (Normative)](#2-swm-schema-definition-normative)
- [3. Relationship and Topology Enforcement](#3-relationship-and-topology-enforcement)
- [4. Confidence Computation and Propagation](#4-confidence-computation-and-propagation)
- [5. Event Log and Causality Model](#5-event-log-and-causality-model)
- [6. CRDT Implementation Rules](#6-crdt-implementation-rules)
- [7. Session and Wire Protocol](#7-session-and-wire-protocol)
- [8. Discovery and Federation Protocol Details](#8-discovery-and-federation-protocol-details)
- [9. Spatial URL Resolution Implementation](#9-spatial-url-resolution-implementation)
- [10. OSM Prior Ingestion and Supersession](#10-osm-prior-ingestion-and-supersession)
- [11. Persistence, Migration, and Recovery](#11-persistence-migration-and-recovery)

## 1. Purpose

### 1.1 Objective
1.1.1 Specify concrete data structures, merge algorithms, and protocol flows required to implement SOS convergence and interoperability.

## 2. SWM Schema Definition (Normative)

### 2.1 Core Entity Shape
2.1.1 Every entity MUST conform to:
```json
{
  "id": "uuid",
  "type": "room|doorway|surface|object|connector|asset",
  "version": 0,
  "lifecycle_state": "ghost|anchored|aligned|verified|deprecated",
  "geometry_ref": "asset://...",
  "semantic_tags": ["string"],
  "affordances": [{"name":"sit","confidence":0.0}],
  "confidence": {
    "geometry": 0.0,
    "semantic": 0.0,
    "pose": 0.0,
    "traversal": 0.0,
    "source_trust": 0.0
  },
  "provenance": {
    "source_type": "capture|import|osm|manual",
    "source_id": "string",
    "created_at_ns": 0,
    "updated_at_ns": 0
  }
}
```

### 2.2 Room Entity Extensions
2.2.1 Room entities MUST include:
- local coordinate frame id
- neighbor doorway ids
- occupancy and accessibility hints
- geospatial anchor estimate

### 2.3 Doorway Entity Extensions
2.3.1 Doorways MUST include exactly two endpoints unless marked as `multi_portal = true`.  
2.3.2 Doorways MUST carry traversal confidence and observed transition histogram.

## 3. Relationship and Topology Enforcement

### 3.1 Edge Types
3.1.1 `connected_via` edges connect room <-> doorway <-> room.  
3.1.2 `adjacent_to` edges represent inferred adjacency and MUST NOT replace explicit doorway edges.

### 3.2 Topology Constraints
3.2.1 Graph validator MUST run on each topology mutation.  
3.2.2 Invalid graph updates MUST be quarantined as proposed updates, not applied directly.

### 3.3 Duplicate Collapse Rule
3.3.1 Two entities MAY be merged only if:
- geometric overlap threshold met
- semantic similarity threshold met
- conflict risk below policy threshold

## 4. Confidence Computation and Propagation

### 4.1 Per-Observation Confidence
4.1.1 Observation confidence vector is computed from:
- source quality prior
- tracking stability window
- geometric consistency residual
- cross-source agreement

### 4.2 Propagation Rule
4.2.1 When merging observation `o` into entity `e`, use weighted incremental update:
```text
conf_new = clamp((w_e * conf_e + w_o * conf_o) / (w_e + w_o), 0, 1)
```
4.2.2 `w_o` MUST incorporate source trust and recency decay.

### 4.3 Outlier Policy
4.3.1 Hard outliers (`distance > D_hard` or impossible topology) are rejected.  
4.3.2 Soft outliers are retained as alternate hypotheses for analyst review.

## 5. Event Log and Causality Model

### 5.1 Event Envelope
5.1.1 All events MUST include:
- `event_id`
- `event_type`
- `entity_id`
- `actor_id`
- `timestamp_ns`
- `causal_refs`
- `payload`

### 5.2 Causal Ordering
5.2.1 Each node maintains lamport clock per session.  
5.2.2 Merge engine uses vector or hybrid logical clock metadata for conflict ordering.

### 5.3 Idempotency
5.3.1 Event application MUST be idempotent by `event_id`.  
5.3.2 Duplicate events MUST not change resulting state.

## 6. CRDT Implementation Rules

### 6.1 Field-to-CRDT Mapping
6.1.1 `semantic_tags` -> OR-Set.  
6.1.2 scalar metadata -> LWW register with deterministic tie-breaker (`actor_id`, `event_id`).  
6.1.3 ordered annotations -> sequence CRDT.  
6.1.4 graph edges -> graph CRDT with tombstones.

### 6.2 Tombstone Retention
6.2.1 Tombstones MUST be retained until all known replicas acknowledge compaction watermark.  
6.2.2 Unsafe tombstone GC is forbidden.

### 6.3 Conflict Visibility
6.3.1 Safety-impacting conflicts MUST be preserved and visible to analyst tools.  
6.3.2 Non-safety cosmetic conflicts MAY auto-resolve via deterministic policy.

## 7. Session and Wire Protocol

### 7.1 Handshake Sequence
7.1.1 Client sends `HELLO`:
```json
{
  "type":"HELLO",
  "protocol_version":"1.0",
  "capabilities":{"depth":true,"splat_render":false},
  "auth":{"token":"..."}
}
```
7.1.2 Server replies `HELLO_ACK` with negotiated protocol and feature flags.

### 7.2 Message Types
7.2.1 `OBS_BATCH` for observation uploads.  
7.2.2 `EVENT_DELTA` for state mutation replication.  
7.2.3 `ASSET_CHUNK` for large artifact transfer.  
7.2.4 `POLICY_NOTICE` for safety/permission events.

### 7.3 Reliability Classes
7.3.1 Topology and policy updates MUST use acked channel.  
7.3.2 Telemetry MAY use best-effort stream.

## 8. Discovery and Federation Protocol Details

### 8.1 BLE Advertisement Payload
8.1.1 BLE payload MUST include:
- short node id hash
- protocol version
- pairing hint
- role capability flags

### 8.2 mDNS Service
8.2.1 Service record MUST publish sync endpoint, asset endpoint, and control endpoint.

### 8.3 Federation Registry Sync
8.3.1 Nodes exchange neighbor summaries with:
- region index cell
- trust tier
- route cost hints

## 9. Spatial URL Resolution Implementation

### 9.1 Parser
9.1.1 URL parser MUST support node, region/room, and entity forms.

### 9.2 Resolver Steps
9.2.1 Check local cache index.  
9.2.2 Query local node index.  
9.2.3 Query federation neighbors by cost/proximity.  
9.2.4 Use gateway fallback if unresolved.

### 9.3 Result Semantics
9.3.1 Resolution result MUST include:
- canonical entity/node id
- source node
- confidence of resolution

## 10. OSM Prior Ingestion and Supersession

### 10.1 OSM Ingestion
10.1.1 OSM importer maps buildings/roads/paths to connector and region priors.  
10.1.2 Imported priors MUST be tagged `source_type = osm`.

### 10.2 Supersession Rule
10.2.1 SOS observation-derived state supersedes OSM when:
- confidence exceeds OSM confidence by threshold
- recency and consistency checks pass

### 10.3 Reversibility
10.3.1 Supersession decisions MUST be reversible via audit rollback if later evidence invalidates assumptions.

## 11. Persistence, Migration, and Recovery

### 11.1 Storage Layout
11.1.1 `entities` store (current materialized state).  
11.1.2 `events` store (append-only log).  
11.1.3 `snapshots` store (periodic checkpoint).  
11.1.4 `assets` store (content-addressed binaries).

### 11.2 Migration Rules
11.2.1 Schema migrations MUST be backward compatible for at least one minor release window.  
11.2.2 Migrators MUST be deterministic and idempotent.

### 11.3 Disaster Recovery
11.3.1 Recovery sequence:
- restore latest snapshot
- replay event log from snapshot watermark
- reconcile CRDT tombstones
- verify topology invariants before service admission
