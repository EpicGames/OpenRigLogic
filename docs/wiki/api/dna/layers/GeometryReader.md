# API Reference — `dna/layers/GeometryReader`

---

<!-- ink:api name="GeometryReader" module="dna/layers/GeometryReader" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->
## `class DNAAPI GeometryReader : public virtual DefinitionReader`

Read vertex data (positions and texture coordinates) of the meshes in a rig, without modifying it.

### When to use this

Use it to read mesh geometry from a loaded DNA. Use `getVertexPosition` for convenient per-vertex access. Use the `getVertexPositionXs`, `Ys` and `Zs` accessors for performance-critical bulk access. If you need to write or patch geometry data, use `GeometryWriter` instead.


### Method groups

| Group | Methods |
|-------|---------|
| VertexPosition | getVertexPositionCount, getVertexPosition, getVertexPositionXs, getVertexPositionYs, getVertexPositionZs |
| VertexTextureCoordinate | getVertexTextureCoordinateCount |

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
| `meshIndex` | `std::uint16_t` | required. Position in the zero-indexed array of meshes. Must be less than `getMeshCount`. |
| `vertexIndex` | `std::uint32_t` | required. Position in the zero-indexed array of vertex positions. Must be less than `getVertexPositionCount`. |

### Returns

`Position`, `ConstArrayView<float>` or `std::uint32_t`, depending on the method. These are a vertex position, a view over all X, Y or Z values of a mesh, and an element count.

### Constraints

- `meshIndex` must be less than the value returned by `getMeshCount`.
- `vertexIndex` must be less than the value returned by `getVertexPositionCount`.
- Vertices are sorted by vertex ID.

### Watch out for

- Implementors should inherit from `Reader` itself, not this class.
- The destructor is protected, so you cannot delete an instance through a `GeometryReader` pointer.
- `meshIndex` and `vertexIndex` are not bounds-checked. Passing an out-of-range value is undefined behavior — always validate against `getMeshCount()` and `getVertexPositionCount()` first.
- `getVertexPositionXs`, `getVertexPositionYs`, and `getVertexPositionZs` return a `ConstArrayView<float>` — a non-owning view. The view is only valid while the underlying DNA data object is alive.
- Do not inherit from `GeometryReader` directly. The class-level `@warning` requires implementors to inherit from `Reader` itself, not from this interface.

<!-- ink:api-end name="GeometryReader" -->
