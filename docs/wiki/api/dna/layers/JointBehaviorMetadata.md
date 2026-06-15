# API Reference — `dna/layers/JointBehaviorMetadata`

---

<!-- ink:api name="RotationRepresentation" module="dna/layers/JointBehaviorMetadata" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RotationRepresentation`

Identifies whether joint rotation data is stored as Euler angles or as a quaternion in a DNA file.

### Why this exists

Euler angles and quaternions require fundamentally different parsing and evaluation paths: Euler angles carry an implicit axis order (typically XYZ) and are susceptible to gimbal lock, while quaternions are four-component unit vectors suited for spherical interpolation. Reading code cannot safely pick a path without this tag — branching on `RotationRepresentation` ensures the correct decode and avoids silent data corruption. The two-enumerator design also signals clearly to future contributors that adding a new rotation format requires an explicit version bump rather than a silent convention change.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `EulerAngles` | enumerator | Rotations are stored as three Euler angles; axis order is defined by the DNA format specification. |
| `Quaternion` | enumerator | Rotations are stored as a four-component unit quaternion (X, Y, Z, W). |

### Relationships

- `TranslationRepresentation` — companion enum for the translation component of the same joint transform
- `ScaleRepresentation` — companion enum for the scale component of the same joint transform

<!-- ink:api-end name="RotationRepresentation" -->

<!-- ink:api name="ScaleRepresentation" module="dna/layers/JointBehaviorMetadata" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ScaleRepresentation`

Identifies the storage format used for joint scale data in a DNA file.

### Why this exists

Scale data, like translation data, could in principle be stored in multiple formats as the DNA specification evolves. Representing the current format as a typed enum — rather than a bare integer flag — keeps the parsing contract explicit and makes future extensions backward-compatible for code that switches on this value. It mirrors `TranslationRepresentation` in structure and intent, completing the three-component joint transform representation trio.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Vector` | enumerator | Scale is stored as a three-component vector (X, Y, Z). |

### Relationships

- `TranslationRepresentation` — companion enum for the translation component of the same joint transform
- `RotationRepresentation` — companion enum for the rotation component of the same joint transform

<!-- ink:api-end name="ScaleRepresentation" -->

<!-- ink:api name="TranslationRepresentation" module="dna/layers/JointBehaviorMetadata" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `TranslationRepresentation`

Identifies the storage format used for joint translation data in a DNA file.

### Why this exists

A DNA file may support multiple ways to store the same transform component; without an explicit type tag, consumers would need out-of-band knowledge to parse translation data correctly. This enum makes the representation contract machine-readable and forward-compatible — adding a new enumerator in a future version is non-breaking for code that switches on this value. It is defined alongside `RotationRepresentation` and `ScaleRepresentation` so that all three transform axes are governed by the same pattern.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Vector` | enumerator | Translations are stored as a three-component vector (X, Y, Z). |

### Relationships

- `RotationRepresentation` — companion enum for the rotation component of the same joint transform
- `ScaleRepresentation` — companion enum for the scale component of the same joint transform

<!-- ink:api-end name="TranslationRepresentation" -->
