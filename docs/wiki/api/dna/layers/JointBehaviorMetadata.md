# API Reference — `dna/layers/JointBehaviorMetadata`

---

<!-- ink:api name="RotationRepresentation" module="dna/layers/JointBehaviorMetadata" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `RotationRepresentation`

Describes how a joint's rotation values are represented.

### Why this exists
Euler angles and quaternions require fundamentally different parsing and evaluation paths: Euler angles carry an implicit axis order (typically XYZ) and are susceptible to gimbal lock, while quaternions are four-component unit vectors suited for spherical interpolation. Reading code cannot safely pick a path without this tag — branching on `RotationRepresentation` ensures the correct decode and avoids silent data corruption. The two-enumerator design also signals clearly to future contributors that adding a new rotation format requires an explicit version bump rather than a silent convention change.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `EulerAngles` | enumerator | Rotation is stored as Euler angles. |
| `Quaternion` | enumerator | Rotation is stored as a quaternion. |

### Relationships

- `TranslationRepresentation` — *sibling enum for translation*
- `ScaleRepresentation` — *sibling enum for scale*
<!-- ink:api-end name="RotationRepresentation" -->

<!-- ink:api name="ScaleRepresentation" module="dna/layers/JointBehaviorMetadata" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `ScaleRepresentation`

Describes how a joint's scale values are represented.

### Why this exists
Scale data, like translation data, could in principle be stored in multiple formats as the DNA specification evolves. Representing the current format as a typed enum — rather than a bare integer flag — keeps the parsing contract explicit and makes future extensions backward-compatible for code that switches on this value. It mirrors `TranslationRepresentation` in structure and intent, completing the three-component joint transform representation trio.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Vector` | enumerator | Scale is stored as a vector. The only supported representation. |

### Relationships

- `TranslationRepresentation` — *sibling enum for translation*
- `RotationRepresentation` — *sibling enum for rotation*
<!-- ink:api-end name="ScaleRepresentation" -->

<!-- ink:api name="TranslationRepresentation" module="dna/layers/JointBehaviorMetadata" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `TranslationRepresentation`

Describes how a joint's translation values are represented.

### Why this exists
A DNA file may support multiple ways to store the same transform component; without an explicit type tag, consumers would need out-of-band knowledge to parse translation data correctly. This enum makes the representation contract machine-readable and forward-compatible — adding a new enumerator in a future version is non-breaking for code that switches on this value. It is defined alongside `RotationRepresentation` and `ScaleRepresentation` so that all three transform axes are governed by the same pattern.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Vector` | enumerator | Translation is stored as a vector. The only supported representation. |

### Relationships

- `RotationRepresentation` — *sibling enum for rotation*
- `ScaleRepresentation` — *sibling enum for scale*
<!-- ink:api-end name="TranslationRepresentation" -->
