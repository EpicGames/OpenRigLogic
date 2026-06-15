# API Reference — `dna/layers/GeometryReader`

---

<!-- ink:api name="GeometryReader" module="dna/layers/GeometryReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class GeometryReader : public virtual DefinitionReader`

Access all geometry data — vertex positions, texture coordinates, and related mesh data — for a rig's meshes. This is the read-only side of the geometry layer; pair it with `GeometryWriter` when you need to modify mesh data.

### When to use this

Use this when you need to inspect or export mesh geometry from a DNA asset — vertex positions, UV sets, or per-mesh data — without modifying it. If you need to write or patch geometry data, use `GeometryWriter` instead. Do not inherit from `GeometryReader` directly; the class requires implementors to inherit from `Reader` itself.

### Method groups

| Group | Methods |
|-------|---------|
| Vertex positions | `getVertexPositionCount`, `getVertexPosition`, `getVertexPositionXs`, `getVertexPositionYs`, `getVertexPositionZs` |
| Texture coordinates | `getVertexTextureCoordinateCount` |

### Example

```cpp
// Obtain a GeometryReader via a concrete reader (e.g. BinaryStreamReader)
dna::BinaryStreamReader* reader = /* ... */;

// Vertex count for mesh 0
std::uint32_t vCount = reader->getVertexPositionCount(0);

// Read individual vertex positions (convenient path)
for (std::uint32_t i = 0; i < vCount; ++i) {
    dna::Position pos = reader->getVertexPosition(0, i);
    // pos.x, pos.y, pos.z
}

// Bulk read all X values for mesh 0 (performance-critical path)
dna::ConstArrayView<float> xs = reader->getVertexPositionXs(0);
// xs[i] == X coordinate of the i-th vertex
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `meshIndex` | `std::uint16_t` | required — zero-based mesh index. Must be less than `getMeshCount()`. |
| `vertexIndex` | `std::uint32_t` | required — zero-based vertex index within the mesh. Must be less than `getVertexPositionCount(meshIndex)`. Vertices are ordered by vertex ID. |

### Watch out for

- `meshIndex` and `vertexIndex` are not bounds-checked. Passing an out-of-range value is undefined behavior — always validate against `getMeshCount()` and `getVertexPositionCount()` first.
- `getVertexPositionXs`, `getVertexPositionYs`, and `getVertexPositionZs` return a `ConstArrayView<float>` — a non-owning view. The view is only valid while the underlying DNA data object is alive.
- Do not inherit from `GeometryReader` directly. The class-level `@warning` requires implementors to inherit from `Reader` itself, not from this interface.

<!-- ink:api-end name="GeometryReader" -->
