# SOS Spec Part 11: Property Model Rights, Real Estate Binding, and Notarization

## Table of Contents

- [1. Purpose and Legal Design Intent](#1-purpose-and-legal-design-intent)
- [2. Two Ownership Layers](#2-two-ownership-layers)
- [3. Property Model Rights (PMR)](#3-property-model-rights-pmr)
- [4. Real Estate Parcel Binding](#4-real-estate-parcel-binding)
- [5. Notarization and Attestation](#5-notarization-and-attestation)
- [6. Enforcement Architecture](#6-enforcement-architecture)
- [7. Public vs Private Property](#7-public-vs-private-property)
- [8. Creator vs Owner](#8-creator-vs-owner)
- [9. Property Transfer on Real Estate Sale](#9-property-transfer-on-real-estate-sale)
- [10. Relationship to Virtual Objects (Part 9)](#10-relationship-to-virtual-objects-part-9)
- [11. Disputes, Appeals, and Revocation](#11-disputes-appeals-and-revocation)
- [12. Phased Implementation](#12-phased-implementation)
- [13. MCP and Registry Interfaces](#13-mcp-and-registry-interfaces)

## 1. Purpose and Legal Design Intent

### 1.1 Purpose
1.1.1 Define how spatial models of real property are legally and technically bound to **real estate ownership**, distinct from portable virtual object ownership.

### 1.2 Design Intent
1.2.1 If you own a house, you own the **authoritative spatial model of that house** (subject to notarized proof and policy).  
1.2.2 A friend's picture frame placed in your house remains **their** virtual object; it does not transfer your property model rights.  
1.2.3 When you sell the house, the **property model** transfers to the buyer as part of property rights workflow.  
1.2.4 Streets and public areas are owned by the **public property authority**; contributors may be recorded as creators but not as property owners.

### 1.3 Non-Goal
1.3.1 This spec does not replace land-title law or provide legal advice; it defines **system requirements** for binding, notarizing, and enforcing property-model ownership in SOS.

## 2. Two Ownership Layers

### 2.1 Layer A — Property Model Rights (PMR)
2.1.1 Applies to SWM entities representing **real property geometry and fixed spatial truth** tied to a legal parcel.  
2.1.2 Owner: verified property owner or designated public authority.  
2.1.3 Transfers with real estate sale or authority assignment.

### 2.2 Layer B — Portable Virtual Object Ownership
2.2.1 Applies to user-placed virtual objects (picture frames, speakers, screens). See Part 9.  
2.2.2 Owner: individual user who owns the object.  
2.2.3 Does **not** transfer with property sale unless explicitly included in a separate agreement.

### 2.3 Separation Rule
2.3.1 Property model and virtual objects MUST remain separate ownership records.  
2.3.2 Property owner MAY revoke placement permission for virtual objects but MUST NOT seize virtual object ownership without object-owner consent or legal process.

## 3. Property Model Rights (PMR)

### 3.1 PMR Record
3.1.1 Each notarized property model MUST have a PMR record:
```json
{
  "pmr_id": "uuid",
  "property_parcel_id": "legal_parcel_ref",
  "swm_root_entity_id": "room_or_building_uuid",
  "owner_entity_id": "person_or_org_uuid",
  "owner_type": "individual|organization|public_authority",
  "rights_class": "private_residential|commercial|public_right_of_way|mixed",
  "notarization_ref": "attestation_id",
  "effective_from_ns": 0,
  "effective_until_ns": null,
  "status": "pending|active|disputed|transferred|revoked"
}
```

### 3.2 What PMR Covers
3.2.1 Building/room shell geometry and structural semantics.  
3.2.2 Fixed outdoor connectors tied to parcel (driveway, private yard, private structures).  
3.2.3 Accumulated scan history designated as **property-authoritative** for that parcel.

### 3.3 What PMR Does Not Cover
3.3.1 Portable virtual objects placed by guests (Part 9).  
3.3.2 Personal avatars and user inventory.  
3.3.3 Neighbor property models outside parcel boundary.

## 4. Real Estate Parcel Binding

### 4.1 Parcel Reference
4.1.1 PMR MUST bind to a legal parcel identifier (county APN, cadastral id, or jurisdiction-specific equivalent).  
4.1.2 Geospatial footprint MUST align with parcel polygon within configured tolerance.

### 4.2 Binding Workflow
4.2.1 Capture/import produces candidate property model.  
4.2.2 Owner initiates **parcel claim** with evidence package.  
4.2.3 Notarization service validates claim and activates PMR.

### 4.3 Multi-Unit and Shared Property
4.3.1 Condominiums and shared buildings MUST support sub-parcel or unit-level PMR scopes.  
4.3.2 Common areas MAY have separate PMR owned by HOA/management entity.

## 5. Notarization and Attestation

### 5.1 Notarization Objective
5.1.1 Produce a **legally meaningful, tamper-evident attestation** that a given SOS property model is owned by a verified party for a given parcel.

### 5.2 Evidence Package (Minimum)
5.2.1 Property owner identity verification (KYC or title-aligned identity provider).  
5.2.2 Parcel/title reference or authoritative registry lookup result.  
5.2.3 Model fingerprint (hash of authoritative SWM subtree + asset manifest).  
5.2.4 Capture provenance summary and confidence metrics.  
5.2.5 Optional: survey/geotag corroboration, utility bill address match, notary signature.

### 5.3 Attestation Record
5.3.1 Notarization emits signed `PROPERTY_MODEL_ATTESTED` event:
```json
{
  "attestation_id": "uuid",
  "pmr_id": "uuid",
  "model_fingerprint": "sha256:...",
  "parcel_id": "legal_parcel_ref",
  "owner_entity_id": "uuid",
  "issuer_id": "notary_authority_id",
  "issued_at_ns": 0,
  "signature": "...",
  "evidence_refs": ["doc://..."]
}
```

### 5.4 Notarization Providers (Pluggable)
5.4.1 SOS MUST support multiple attestation backends:
- title-company-integrated registry
- government land-record API (where available)
- accredited third-party notary service
- cryptographic timestamping + audit trail (minimum technical baseline)

5.4.2 Jurisdiction-specific legal validity is determined by provider + local law, not by SOS alone.

## 6. Enforcement Architecture

### 6.1 Spatial Authority Node (SAN)
6.1.1 SAN is the policy authority that enforces PMR on nodes serving a parcel.  
6.1.2 SAN validates attestation before accepting a model as **property-authoritative**.

### 6.2 Enforcement Rules
6.2.1 Mutations to property-authoritative geometry REQUIRE PMR holder permission or delegated surveyor license.  
6.2.2 Non-owner contributions are stored as **observations** until accepted/merged by PMR policy.  
6.2.3 Conflicting property claims MUST enter disputed state; no silent overwrite of active PMR.

### 6.3 Runtime Checks
6.3.1 On read: clients MAY display non-authoritative models with visual "provisional/unverified" state.  
6.3.2 On write: policy service MUST reject unauthorized property-authoritative writes.  
6.3.3 On federation sync: remote nodes MUST honor PMR attestation chain from SAN/registry.

### 6.4 Audit and Non-Repudiation
6.4.1 All PMR changes (claim, transfer, revoke, dispute resolution) MUST be append-only auditable events.

## 7. Public vs Private Property

### 7.1 Private Property (Residential/Commercial)
7.1.1 PMR owner is verified deed holder or authorized organization.  
7.1.2 Model of house/building/parcel belongs to property owner.

### 7.2 Public Property (Streets, Sidewalks, Parks, Public Right-of-Way)
7.2.1 PMR owner is public authority entity (city, county, state, federal as applicable).  
7.2.2 Individuals MAY contribute scans/observations but do not gain ownership of public parcel models.

### 7.3 OSM and Public Prior Overlap
7.3.1 OSM may bootstrap public geometry priors.  
7.3.2 Authoritative public PMR supersedes bootstrap priors when notarized public model exists.

## 8. Creator vs Owner

### 8.1 Definitions
8.1.1 **Creator**: party who captured, imported, or materially contributed observations.  
8.1.2 **Owner**: party with PMR for the property parcel model.

### 8.2 Rules
8.2.1 Creator credit MUST be preserved in provenance permanently.  
8.2.2 Creator status does not confer property ownership.  
8.2.3 Owner controls authoritative merge policy for property model on their parcel.  
8.2.4 Public spaces: creator retained, owner remains public authority.

## 9. Property Transfer on Real Estate Sale

### 9.1 Transfer Trigger
9.1.1 Real estate closing event (manual admin trigger in early phases; registry-integrated trigger later).

### 9.2 Transfer Workflow
9.2.1 Seller PMR marked `transferred`.  
9.2.2 New PMR issued to buyer with updated owner_entity_id.  
9.2.3 Model fingerprint and SWM root reference carry forward unless explicit remodel reset requested.  
9.2.4 Attestation re-issued or counter-signed by title/notary provider.

### 9.3 Virtual Objects at Sale
9.3.1 Virtual objects DO NOT auto-transfer with PMR.  
9.3.2 Seller or object owners SHOULD remove or reclaim portable objects before/after closing.  
9.3.3 New property owner MAY revoke future placement permissions in their space.

### 9.4 Escrow and Dispute Window
9.4.1 Optional post-closing dispute window where prior owner read-only access is retained for N days (policy-configurable).

## 10. Relationship to Virtual Objects (Part 9)

### 10.1 Coexistence Model
10.1.1 Property model = fixed spatial truth of real estate (PMR).  
10.1.2 Virtual objects = portable user-owned entities placed within that space.

### 10.2 Example
10.2.1 You own your house PMR.  
10.2.2 Friend owns picture frame virtual object placed on your wall.  
10.2.3 You control property model updates; friend controls frame content and ownership; you control whether placement remains allowed.

### 10.3 Removal vs Ownership
10.3.1 Property owner MAY require removal of placed virtual objects (lease/house rules).  
10.3.2 Removal revokes placement, not object ownership.

## 11. Disputes, Appeals, and Revocation

### 11.1 Dispute Scenarios
11.1.1 Competing claims for same parcel.  
11.1.2 Fraudulent notarization evidence.  
11.1.3 Boundary overlap between neighbors.  
11.1.4 Tenant vs landlord authority conflicts.

### 11.2 Dispute State
11.2.1 PMR enters `disputed`; authoritative writes frozen except by dispute resolver role.  
11.2.2 Non-authoritative observations may continue with clear labeling.

### 11.3 Revocation
11.3.1 Attestation MAY be revoked by issuer or governance board with documented cause.  
11.3.2 Revoked PMR MUST downgrade model to provisional until re-notarized.

## 12. Phased Implementation

### 12.1 Phase 0 (Local Room MVP)
12.1.1 PMR not required; local SWM only, no legal binding.

### 12.2 Phase 1 (Single-Property Claim)
12.2.1 Manual parcel claim + admin attestation for early adopters.  
12.2.2 SAN enforcement on single home node.

### 12.3 Phase 2 (Notary Integration)
12.3.1 Pluggable notary providers, evidence packages, signed attestations.  
12.3.2 Property transfer workflow on sale.

### 12.4 Phase 3 (Public Property Registry)
12.4.1 Public authority PMR for streets and civic spaces.  
12.4.2 Creator/owner separation enforced at scale.

## 13. MCP and Registry Interfaces

### 13.1 Registry Query
13.1.1 `get_pmr(parcel_id)`  
13.1.2 `get_attestation(attestation_id)`  
13.1.3 `verify_model_fingerprint(pmr_id, fingerprint)`

### 13.2 Claim and Transfer
13.2.1 `submit_property_claim(evidence_package)`  
13.2.2 `initiate_property_transfer(pmr_id, buyer_entity_id, closing_ref)`  
13.2.3 `resolve_dispute(pmr_id, resolution)`

### 13.3 Policy Simulation
13.3.1 `simulate_write_authority(actor_id, entity_id, mutation_type)`  
13.3.2 `list_portable_objects_in_property(pmr_id)`
