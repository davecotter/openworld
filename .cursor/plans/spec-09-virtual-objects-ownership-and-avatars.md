# SOS Spec Part 9: Virtual Objects, Ownership, and Avatars

## Table of Contents

- [1. Purpose and Scope](#1-purpose-and-scope)
- [2. Virtual Object Model](#2-virtual-object-model)
- [3. Ownership and Permission Model](#3-ownership-and-permission-model)
- [4. Handoff and Transfer Protocol](#4-handoff-and-transfer-protocol)
- [5. Placement Rules and Anchoring](#5-placement-rules-and-anchoring)
- [6. Content Rights and Mutable Media](#6-content-rights-and-mutable-media)
- [7. Object Type Specifications](#7-object-type-specifications)
- [8. Create and Import Workflows](#8-create-and-import-workflows)
- [9. Event Types and Sync Semantics](#9-event-types-and-sync-semantics)
- [10. Security, Abuse, and Audit](#10-security-abuse-and-audit)
- [11. Avatar System Roadmap](#11-avatar-system-roadmap)
- [12. MCP Tools for Virtual Objects](#12-mcp-tools-for-virtual-objects)

## 1. Purpose and Scope

### 1.1 Purpose
1.1.1 Define how users create or import virtual objects, own them, transfer them, place them in spaces, and mutate their content under enforceable permissions.

### 1.2 In Scope
1.2.1 User-owned virtual objects (picture frames, speakers, video screens, and extensible types).  
1.2.2 Ownership transfer via handoff between users.  
1.2.3 Placement on surfaces (walls, shelves, floors) with space and object permission checks.  
1.2.4 Owner-controlled content updates (for example changing the image in a owned picture frame).  
1.2.5 Avatar representation with a phased roadmap including future VRChat-compatible imports.

### 1.3 Out of Scope (Initial)
1.3.1 Real-money marketplace transactions (future extension).  
1.3.2 Full DRM for third-party media beyond policy and access control.  
1.3.3 Real estate Property Model Rights (PMR) — see Part 11; virtual objects here are portable and separate from parcel-bound models.

## 2. Virtual Object Model

### 2.1 Entity Class
2.1.1 Virtual objects are first-class SWM entities with type `virtual_object`.  
2.1.2 Each virtual object MUST reference:
- geometry/asset for frame/device shell
- optional mutable content asset(s)
- ownership record
- placement anchor (if placed)
- capability profile (`picture_frame`, `audio_speaker`, `video_screen`, `generic`)

### 2.2 Canonical Virtual Object Shape
2.2.1 Normative JSON:
```json
{
  "id": "uuid",
  "type": "virtual_object",
  "object_kind": "picture_frame|audio_speaker|video_screen|generic",
  "owner_id": "user_uuid",
  "placement": {
    "state": "held|placed|stored",
    "space_id": "room_uuid_or_null",
    "anchor_id": "surface_uuid_or_null",
    "transform_world": [0,0,0,0,0,0,0]
  },
  "content": {
    "primary_ref": "asset://...",
    "content_type": "image|video|audio_stream|url|none",
    "revision": 0
  },
  "permissions": {
    "can_handoff": true,
    "can_place_in_space": "owner_only|owner_or_space_delegate",
    "can_edit_content": "owner_only"
  },
  "provenance": {
    "created_by": "user_uuid",
    "created_at_ns": 0,
    "source": "created|imported"
  }
}
```

### 2.3 Held vs Placed vs Stored
2.3.1 `held`: object is attached to a user avatar/hand inventory slot.  
2.3.2 `placed`: object is anchored to a surface in a room/space.  
2.3.3 `stored`: object is owned but not currently rendered in world (inventory/backpack).

## 3. Ownership and Permission Model

### 3.1 Ownership Rule
3.1.1 Exactly one `owner_id` MUST exist per virtual object at any time.  
3.1.2 Ownership transfer MUST be explicit via signed transfer event.

### 3.2 Permission Layers
3.2.1 **Object ownership permissions** (who can edit content, hand off, delete object).  
3.2.2 **Space permissions** (who can place objects in a room/building).  
3.2.3 **Interaction permissions** (who can pick up, move, or use object affordances).

### 3.3 Placement Authorization Matrix
3.3.1 User MAY place object on wall/surface if:
- user is `owner_id`, OR
- user has explicit `place_object` delegate permission from space owner/admin.

3.3.2 User MUST NOT place object if neither condition is true.

### 3.4 Content Edit Authorization
3.4.1 Only current object owner MAY mutate object content (unless delegated by owner policy).  
3.4.2 Example: owner of picture frame MAY replace image at any time.

### 3.5 Space Owner vs Object Owner
3.5.1 Space owner controls where objects may be placed in that space.  
3.5.2 Object owner controls object content and transfer.  
3.5.3 Space owner MUST NOT mutate object content without object owner permission.

## 4. Handoff and Transfer Protocol

### 4.1 Handoff Intent
4.1.1 Person A initiates handoff targeting Person B while both are in interaction range (or remote handoff if policy allows).

### 4.2 Handoff State Machine
4.2.1 States:
- `idle`
- `offer_pending` (A offered, waiting B)
- `accept_pending` (B accepted, server validating)
- `completed`
- `rejected`
- `expired`

### 4.3 Transfer Sequence
4.3.1 A emits `HANDOFF_OFFER(object_id, to_user_id)`.  
4.3.2 B emits `HANDOFF_ACCEPT(offer_id)`.  
4.3.3 Policy service validates:
- A is current owner
- B is eligible recipient
- object state allows transfer (`held` or policy-permitted `placed`)
4.3.4 System emits `OWNERSHIP_TRANSFERRED` event:
- old owner -> A
- new owner -> B
- transfer timestamp and causal refs

### 4.4 Post-Transfer Behavior
4.4.1 If object was `held` by A, it transitions to `held` by B.  
4.4.2 If object was `placed`, default behavior is transition to `held` by B unless B explicitly keeps placement (requires space permission re-check).

### 4.5 Non-Repudiation
4.5.1 Transfer events MUST be signed by server/policy authority and auditable.

## 5. Placement Rules and Anchoring

### 5.1 Surface Eligibility
5.1.1 Placement target MUST be a valid SWM surface with confidence above placement threshold.  
5.1.2 Wall placement uses surface normal + bounds validation to prevent impossible placement.

### 5.2 Placement Operation
5.2.1 Client sends `PLACE_OBJECT` with:
- object_id
- space_id
- anchor_surface_id
- transform_world
5.2.2 Policy service checks object ownership/delegation + space permission.  
5.2.3 On success, object state becomes `placed`.

### 5.3 Move and Remove
5.3.1 Owner MAY move own placed object if space policy allows.  
5.3.2 Non-owner MUST NOT move object unless delegated.  
5.3.3 Pickup operation transitions `placed` -> `held` for authorized actor.

### 5.4 Placement Persistence
5.4.1 Placement transform is replicated through CRDT/event log and visible to all authorized viewers in that space.

## 6. Content Rights and Mutable Media

### 6.1 Content Ownership
6.1.1 Object shell and object content rights are tied to object owner unless licensed otherwise.

### 6.2 Content Update Flow
6.2.1 Owner sends `UPDATE_OBJECT_CONTENT(object_id, content_ref, content_type)`.  
6.2.2 System increments `content.revision` and broadcasts update to all viewers.

### 6.3 Media Types
6.3.1 Picture frame: image or image sequence.  
6.3.2 Video screen: stream URL or hosted video asset.  
6.3.3 Audio speaker: stream URL or hosted audio asset.

### 6.4 Content Validation
6.4.1 Content uploads MUST pass size/type policy checks.  
6.4.2 Remote URLs MUST pass allowlist/safety scanning policy where enabled.

## 7. Object Type Specifications

### 7.1 Picture Frame
7.1.1 Affordances: `pickup`, `handoff`, `place_on_wall`, `replace_image`.  
7.1.2 Owner MAY replace image at any time.  
7.1.3 Non-owner viewers see updates in real time.

### 7.2 Audio Speaker
7.2.1 Affordances: `pickup`, `handoff`, `place`, `play`, `pause`, `set_stream`.  
7.2.2 Owner controls stream/source selection.  
7.2.3 Space policy MAY cap audio playback permissions for non-owners.

### 7.3 Video Screen
7.3.1 Affordances: `pickup`, `handoff`, `place`, `set_stream`, `set_loop`.  
7.3.2 Owner controls displayed media source.  
7.3.3 Render clients MUST support fallback poster frame when stream unavailable.

### 7.4 Generic Extensible Object (TBD)
7.4.1 Uses capability profile registry for future object classes.  
7.4.2 New object kinds MUST declare affordances and permission defaults.

## 8. Create and Import Workflows

### 8.1 Create New Virtual Object
8.1.1 User selects object template (frame/speaker/screen).  
8.1.2 User assigns initial content (optional).  
8.1.3 System creates `virtual_object` with creator as owner.

### 8.2 Import Existing Virtual Object Asset
8.2.1 User imports external asset package (mesh/material/content bundle).  
8.2.2 Import adapter normalizes to SOS virtual object schema.  
8.2.3 Imported object receives provenance tag and owner assignment to importing user.

### 8.3 Inventory Model
8.3.1 Owned objects not placed in world reside in user inventory (`stored`).  
8.3.2 Inventory is queryable and syncs across user sessions/devices.

## 9. Event Types and Sync Semantics

### 9.1 Required Event Types
9.1.1 `VIRTUAL_OBJECT_CREATED`  
9.1.2 `VIRTUAL_OBJECT_IMPORTED`  
9.1.3 `HANDOFF_OFFERED`  
9.1.4 `HANDOFF_ACCEPTED`  
9.1.5 `OWNERSHIP_TRANSFERRED`  
9.1.6 `OBJECT_PLACED`  
9.1.7 `OBJECT_MOVED`  
9.1.8 `OBJECT_PICKED_UP`  
9.1.9 `OBJECT_CONTENT_UPDATED`  
9.1.10 `OBJECT_DELETED`

### 9.2 Convergence Rules
9.2.1 Ownership changes are authoritative single-owner events (not OR-set).  
9.2.2 Placement transforms use deterministic conflict policy with audit trail for contested moves.

## 10. Security, Abuse, and Audit

### 10.1 Authorization Enforcement
10.1.1 All object mutations MUST pass policy service authorization checks server-side.  
10.1.2 Client-side permission checks are advisory only.

### 10.2 Anti-Abuse Controls
10.2.1 Rate limits on handoff/placement/content update actions.  
10.2.2 Report/block workflow for malicious media or harassment objects.

### 10.3 Audit Trail
10.3.1 Every ownership and permission-sensitive event MUST be auditable with actor, timestamp, and object id.

## 11. Avatar System Roadmap

### 11.1 Phase 1 (MVP): Simple Avatars
11.1.1 Minimal humanoid/proxy avatar with head and hand alignment.  
11.1.2 Supports first-person, over-the-shoulder, and remote avatar locomotion.  
11.1.3 Handoff interactions use simple hand/proximity affordances.

### 11.2 Phase 2: Customization
11.2.1 Basic appearance parameters (color/body scale/accessories).  
11.2.2 Persistent avatar profile bound to user identity.

### 11.3 Phase 3: Imported Avatars (VRChat-Compatible Ecosystem)
11.3.1 Support import of avatars from VRChat-compatible creators/pipelines where licensing and platform policy allow.  
11.3.2 Import adapter normalizes rig/bone mapping into SOS avatar runtime contract.  
11.3.3 Fallback to simplified avatar when imported rig exceeds device capability budget.

### 11.4 Avatar Import Requirements
11.4.1 Normalize to SOS avatar asset schema (mesh, materials, rig, animation set).  
11.4.2 Validate performance budget and safety constraints before activation.  
11.4.3 Preserve provenance and creator attribution metadata.

## 12. MCP Tools for Virtual Objects

### 12.1 Query Tools
12.1.1 `list_user_objects(user_id)`  
12.1.2 `get_object_permissions(object_id)`  
12.1.3 `get_space_placement_policy(space_id)`

### 12.2 Mutation Tools
12.2.1 `create_virtual_object(template, content_ref?)`  
12.2.2 `offer_handoff(object_id, to_user_id)`  
12.2.3 `accept_handoff(offer_id)`  
12.2.4 `place_object(object_id, space_id, surface_id, transform)`  
12.2.5 `update_object_content(object_id, content_ref, content_type)`

### 12.3 Policy Simulation Tools
12.3.1 `simulate_handoff(object_id, from_user, to_user)`  
12.3.2 `simulate_placement(object_id, user_id, space_id, surface_id)`
