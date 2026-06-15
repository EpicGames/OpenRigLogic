# API Reference — `dna/layers/Geometry`

---

<!-- ink:api name="Delta" module="dna/layers/Geometry" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Delta`

Semantic alias for `Vector3` that marks a value as a blend-shape displacement rather than a position or direction.

### Why this exists

Blend-shape deltas are relative offsets applied on top of a base-mesh position — they are not absolute locations, and they should not be treated as directions requiring normalization. Using `Delta` instead of `Vector3` at API boundaries makes this distinction explicit, prevents accidental unit-normalization, and identifies which attribute arrays hold morph-target data versus base geometry.

### Relationships

- `Position` — parallel alias for `Vector3`; holds absolute vertex positions to which deltas are added
- `Normal` — parallel alias for `Vector3`; holds surface normals

<!-- ink:api-end name="Delta" -->

<!-- ink:api name="Normal" module="dna/layers/Geometry" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Normal`

Semantic alias for `Vector3` that marks a value as a surface normal direction rather than a position or displacement.

### Why this exists

Surface normals must be interpreted as unit-length direction vectors — normalizing them incorrectly or treating them as positional data produces rendering artifacts. This alias documents that intent at the type level. Together with `Position` and `Delta`, it forms a set of three semantically distinct `Vector3` aliases that give geometry attribute arrays unambiguous meaning without introducing runtime cost.

### Relationships

- `Position` — parallel alias for `Vector3`; used for vertex positions
- `Delta` — parallel alias for `Vector3`; used for blend-shape displacements
- `VertexLayout` — the `normal` field indexes into an array of `Normal` values

<!-- ink:api-end name="Normal" -->

<!-- ink:api name="Position" module="dna/layers/Geometry" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Position`

Semantic alias for `Vector3` that marks a value as a spatial position rather than a direction or displacement.

### Why this exists

All three geometric attribute types — `Position`, `Normal`, and `Delta` — share the same underlying `Vector3` representation, but carry different mathematical meanings. Using named aliases instead of raw `Vector3` makes function signatures and struct fields self-documenting and prevents passing a surface normal where a vertex position is expected. `Position` specifically identifies an absolute point in 3D space, as opposed to `Normal` (unit direction) or `Delta` (relative displacement).

### Relationships

- `Normal` — parallel alias for `Vector3`; used for surface directions, not positions
- `Delta` — parallel alias for `Vector3`; used for blend-shape displacements
- `VertexLayout` — the `position` field indexes into an array of `Position` values

<!-- ink:api-end name="Position" -->

<!-- ink:api name="TextureCoordinate" module="dna/layers/Geometry" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `TextureCoordinate`

A UV pair that locates a point in 2D texture space for a mesh vertex.

### Why this exists

Raw float pairs lack semantic meaning when stored alongside positions and normals. This struct names the two components `u` and `v` so code that reads or writes texture coordinates is self-documenting. It is consumed by `VertexLayout`, which stores an index into an array of `TextureCoordinate` values rather than embedding the UV directly in each vertex record.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `u` | `float` | required — horizontal texture axis coordinate in [0, 1] normalized space |
| `v` | `float` | required — vertical texture axis coordinate in [0, 1] normalized space |

### Relationships

- `VertexLayout` — holds a `textureCoordinate` index that references a slot in the `TextureCoordinate` attribute array
- `Position` — parallel attribute type; combined with `TextureCoordinate` and `Normal` via `VertexLayout`
- `Normal` — parallel attribute type used alongside this in the same vertex assembly

<!-- ink:api-end name="TextureCoordinate" -->

<!-- ink:api name="VertexLayout" module="dna/layers/Geometry" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `VertexLayout`

Index triple that assembles a logical mesh vertex from three separate attribute arrays — position, texture coordinate, and normal.

### Why this exists

DNA geometry stores positions, texture coordinates, and normals in separate indexed arrays rather than as an interleaved per-vertex record. This avoids duplicating shared attribute values across vertices that differ in only one component (common when UV seams split a shared position into multiple texture-space vertices). `VertexLayout` is the indirection record that names the three per-vertex indices, making explicit which slot in each attribute array contributes to a given logical vertex.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `position` | `std::uint32_t` | required — zero-based index into the mesh's `Position` attribute array |
| `textureCoordinate` | `std::uint32_t` | required — zero-based index into the mesh's `TextureCoordinate` attribute array |
| `normal` | `std::uint32_t` | required — zero-based index into the mesh's `Normal` attribute array |

### Relationships

- `Position` — attribute type indexed by the `position` field
- `TextureCoordinate` — attribute type indexed by the `textureCoordinate` field
- `Normal` — attribute type indexed by the `normal` field

<!-- ink:api-end name="VertexLayout" -->
