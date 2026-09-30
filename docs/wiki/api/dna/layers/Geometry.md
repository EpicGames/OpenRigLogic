# API Reference — `dna/layers/Geometry`

---

<!-- ink:api name="Delta" module="dna/layers/Geometry" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `Delta`

Type alias for `Vector3` that represents a per-vertex offset.

### Why this exists

Blend-shape deltas are relative offsets applied on top of a base-mesh position — they are not absolute locations, and they should not be treated as directions requiring normalization. Using `Delta` instead of `Vector3` at API boundaries makes this distinction explicit, prevents accidental unit-normalization, and identifies which attribute arrays hold morph-target data versus base geometry.

`Delta` labels a `Vector3` as an offset rather than an absolute position. Like `Position` and `Normal`, it is only an alias, so the compiler treats them as the same type.

### Construction

```cpp
dna::Delta d = dna::Vector3{0.0f, 0.02f, 0.0f};
```

### Relationships

- `Position` — *sibling alias of Vector3*
- `Normal` — *sibling alias of Vector3*
- `Position` — parallel alias for `Vector3`; holds absolute vertex positions to which deltas are added
- `Normal` — parallel alias for `Vector3`; holds surface normals
<!-- ink:api-end name="Delta" -->

<!-- ink:api name="Normal" module="dna/layers/Geometry" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `Normal`

Type alias for `Vector3` that represents a vertex normal.

### Why this exists

Surface normals must be interpreted as unit-length direction vectors — normalizing them incorrectly or treating them as positional data produces rendering artifacts. This alias documents that intent at the type level. Together with `Position` and `Delta`, it forms a set of three semantically distinct `Vector3` aliases that give geometry attribute arrays unambiguous meaning without introducing runtime cost.

`Normal` labels a `Vector3` as a normal direction. The compiler does not enforce the distinction, since it is an alias of `Vector3`.

### Construction

```cpp
dna::Normal n = dna::Vector3{0.0f, 0.0f, 1.0f};
```

### Relationships

- `Position` — *sibling alias of Vector3*
- `Delta` — *sibling alias of Vector3*
- `VertexLayout` — *holds an index into the normal array*
- `Delta` — parallel alias for `Vector3`; used for blend-shape displacements
<!-- ink:api-end name="Normal" -->

<!-- ink:api name="Position" module="dna/layers/Geometry" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `Position`

Type alias for `Vector3` that represents a vertex position.

Semantic alias for `Vector3` that marks a value as a spatial position rather than a direction or displacement.

### Why this exists

All three geometric attribute types — `Position`, `Normal`, and `Delta` — share the same underlying `Vector3` representation, but carry different mathematical meanings. Using named aliases instead of raw `Vector3` makes function signatures and struct fields self-documenting and prevents passing a surface normal where a vertex position is expected. `Position` specifically identifies an absolute point in 3D space, as opposed to `Normal` (unit direction) or `Delta` (relative displacement).

### Construction

```cpp
dna::Position p = dna::Vector3{0.0f, 1.5f, 0.0f};
```

### Relationships

- `Normal` — *sibling alias of Vector3*
- `Delta` — *sibling alias of Vector3*
- `VertexLayout` — *holds an index into the position array*
- `Normal` — parallel alias for `Vector3`; used for surface directions, not positions
- `Delta` — parallel alias for `Vector3`; used for blend-shape displacements
<!-- ink:api-end name="Position" -->

<!-- ink:api name="TextureCoordinate" module="dna/layers/Geometry" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `TextureCoordinate`

A 2D UV coordinate for a vertex.

A UV pair that locates a point in 2D texture space for a mesh vertex.

### Why this exists

Raw float pairs lack semantic meaning when stored alongside positions and normals. This struct names the two components `u` and `v` so code that reads or writes texture coordinates is self-documenting. It is consumed by `VertexLayout`, which stores an index into an array of `TextureCoordinate` values rather than embedding the UV directly in each vertex record.

`TextureCoordinate` gives UV pairs a named type instead of two loose floats. It keeps `u` and `v` together, and keeps them distinct from the 3D `Position`, `Normal` and `Delta` types.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `u` | `float` | required. Horizontal texture coordinate. |
| `v` | `float` | required. Vertical texture coordinate. |

### Construction

```cpp
dna::TextureCoordinate uv{0.25f, 0.75f};
```

### Relationships

- `VertexLayout` — *holds an index into the texture coordinate array*
- `Normal` — parallel attribute type used alongside this in the same vertex assembly
- `VertexLayout` — holds a `textureCoordinate` index that references a slot in the `TextureCoordinate` attribute array
<!-- ink:api-end name="TextureCoordinate" -->

<!-- ink:api name="VertexLayout" module="dna/layers/Geometry" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `VertexLayout`

Describes one vertex as a set of indices into the position, texture coordinate and normal arrays.

### Why this exists

DNA geometry stores positions, texture coordinates, and normals in separate indexed arrays rather than as an interleaved per-vertex record. This avoids duplicating shared attribute values across vertices that differ in only one component (common when UV seams split a shared position into multiple texture-space vertices). `VertexLayout` is the indirection record that names the three per-vertex indices, making explicit which slot in each attribute array contributes to a given logical vertex.

`VertexLayout` stores indices instead of copies of the data. Vertices can then share positions, UVs and normals.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `position` | `std::uint32_t` | required. Index into the vertex positions. |
| `textureCoordinate` | `std::uint32_t` | required. Index into the texture coordinates. |
| `normal` | `std::uint32_t` | required. Index into the vertex normals. |

### Construction

```cpp
dna::VertexLayout layout{12u, 12u, 7u};
```

### Relationships

- `Position` — *target of the position index*
- `TextureCoordinate` — *target of the textureCoordinate index*
- `Normal` — *target of the normal index*
- `Position` — attribute type indexed by the `position` field
- `TextureCoordinate` — attribute type indexed by the `textureCoordinate` field
- `Normal` — attribute type indexed by the `normal` field
<!-- ink:api-end name="VertexLayout" -->
