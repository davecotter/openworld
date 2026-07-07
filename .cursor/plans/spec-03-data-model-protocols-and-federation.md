# SOS Spec Part 3: Data Model, Protocols, and Federation

## Table of Contents

- [1. SWM Schema Baseline](#1-swm-schema-baseline)
- [2. Spatial Relationships and Topology](#2-spatial-relationships-and-topology)
- [3. Confidence Model](#3-confidence-model)
- [4. Event Model](#4-event-model)
- [5. CRDT Strategy](#5-crdt-strategy)
- [6. Wire Protocol](#6-wire-protocol)
- [7. Discovery Protocols](#7-discovery-protocols)
- [8. Spatial URL and Addressing](#8-spatial-url-and-addressing)
- [9. Federation Rules](#9-federation-rules)
- [10. Geospatial Routing and Radius Policies](#10-geospatial-routing-and-radius-policies)
- [11. OSM Integration Rules](#11-osm-integration-rules)

## 1. SWM Schema Baseline

### 1.1 Required Entity Types
1.1.1 Room  
1.1.2 Doorway/portal  
1.1.3 Surface  
1.1.4 Object  
1.1.5 Outdoor connector (street/tree/bridge/sidewalk)  
1.1.6 Asset reference

### 1.2 Required Shared Fields
1.2.1 `id` (stable).  
1.2.2 `type`.  
1.2.3 `geometry_ref`.  
1.2.4 `semantic_tags`.  
1.2.5 `affordances`.  
1.2.6 `confidence`.  
1.2.7 `provenance`.  
1.2.8 `lifecycle_state`.

## 2. Spatial Relationships and Topology

### 2.1 Relationship Types
2.1.1 `adjacent_to`  
2.1.2 `contains`  
2.1.3 `connected_via`  
2.1.4 `overlaps_with`  
2.1.5 `aliases`

### 2.2 Topology Invariants
2.2.1 Doorways connect exactly two traversable spaces unless explicitly modeled as multi-portal hubs.  
2.2.2 Relationship confidence is independent from entity confidence.  
2.2.3 Contradictory edges are retained until reconciliation resolves them.

## 3. Confidence Model

### 3.1 Confidence Vector
3.1.1 `geometry_confidence`  
3.1.2 `semantic_confidence`  
3.1.3 `pose_confidence`  
3.1.4 `traversal_confidence`  
3.1.5 `source_trust`

### 3.2 Source Weighting
3.2.1 Weights vary by device class and observed reliability history.  
3.2.2 Multi-source agreement increases confidence non-linearly.

### 3.3 Outlier Handling
3.3.1 Hard outlier rejection for impossible transforms.  
3.3.2 Soft damping for noisy but plausible updates.

## 4. Event Model

### 4.1 Event Envelope
4.1.1 `event_id`  
4.1.2 `event_type`  
4.1.3 `entity_id`  
4.1.4 `payload`  
4.1.5 `actor_id`  
4.1.6 `timestamp`  
4.1.7 `causal_refs`

### 4.2 Canonical Event Types
4.2.1 `OBSERVATION_ADDED`  
4.2.2 `ENTITY_CREATED`  
4.2.3 `ENTITY_UPDATED`  
4.2.4 `ENTITY_MERGED`  
4.2.5 `EDGE_CREATED`  
4.2.6 `EDGE_REWEIGHTED`  
4.2.7 `ROOM_ALIGNMENT_COMPUTED`  
4.2.8 `LIFECYCLE_STATE_CHANGED`

## 5. CRDT Strategy

### 5.1 Mapping
5.1.1 OR-Set for tags and set-like properties.  
5.1.2 LWW-register for scalar fields with deterministic tiebreakers.  
5.1.3 Sequence CRDT for ordered annotations and user narratives.  
5.1.4 Graph CRDT for edges and topological structures.

### 5.2 Conflict Semantics
5.2.1 Preserve conflicting hypotheses if they impact safety or routing.  
5.2.2 Collapse duplicates only with sufficient geometric and semantic confidence.

## 6. Wire Protocol

### 6.1 Session Handshake
6.1.1 Exchange protocol version, capability descriptor, and auth claims.

### 6.2 Message Envelope
6.2.1 `message_type`  
6.2.2 `session_id`  
6.2.3 `sequence_number`  
6.2.4 `payload_encoding`  
6.2.5 `payload`

### 6.3 Reliability Classes
6.3.1 Must-ack for mutations and safety-critical state.  
6.3.2 Best-effort for telemetry and ephemeral hints.

## 7. Discovery Protocols

### 7.1 BLE Discovery
7.1.1 Advertises short identity token, protocol version, and role capabilities.

### 7.2 mDNS Discovery
7.2.1 Advertises local service endpoints for sync, asset fetch, and control.

### 7.3 Remote Discovery
7.3.1 Federation registry announces known neighboring nodes and gateway routes.

## 8. Spatial URL and Addressing

### 8.1 URL Forms
8.1.1 `spatial://node/{node_id}`  
8.1.2 `spatial://region/{region_id}/room/{room_id}`  
8.1.3 `spatial://entity/{entity_id}?view={pose}`

### 8.2 Resolution Order
8.2.1 Local cache.  
8.2.2 Local node.  
8.2.3 Neighbor node.  
8.2.4 Federated gateway.

## 9. Federation Rules

### 9.1 Trust Boundary Rules
9.1.1 Nodes exchange signed claims and trust metadata.  
9.1.2 Mutation rights are scoped by policy and identity tier.

### 9.2 Replication Scope Rules
9.2.1 High-relevance nearby state replicates eagerly.  
9.2.2 Far-field state replicates lazily or on demand.

## 10. Geospatial Routing and Radius Policies

### 10.1 Radius Constraints
10.1.1 Direct peer links require geodesic distance below policy threshold.  
10.1.2 Exception routes require federation approval and relay topology.

### 10.2 Earth Geometry
10.2.1 Routing and partitioning are Earth-native, sphere/geoid aware.  
10.2.2 Coordinate transformations must preserve geodetic consistency.

### 10.3 Indexing
10.3.1 H3/S2 evaluation is required prior to final production lock.

## 11. OSM Integration Rules

### 11.1 Prior Usage
11.1.1 OSM is used as initial structural prior.

### 11.2 Supersession
11.2.1 SOS observations supersede OSM where confidence and recency justify replacement.

### 11.3 Provenance
11.3.1 All OSM-derived entities retain source provenance for auditability.

## 12. Detailed Implementation References

### 12.1 Normative Schema and Protocol Details
12.1.1 Full schema definitions: `spec-06-swm-sync-and-protocol-implementation.md#2-swm-schema-definition-normative`  
12.1.2 CRDT implementation rules: `spec-06-swm-sync-and-protocol-implementation.md#6-crdt-implementation-rules`  
12.1.3 Session/wire protocol details: `spec-06-swm-sync-and-protocol-implementation.md#7-session-and-wire-protocol`  
12.1.4 Discovery/federation payload details: `spec-06-swm-sync-and-protocol-implementation.md#8-discovery-and-federation-protocol-details`
