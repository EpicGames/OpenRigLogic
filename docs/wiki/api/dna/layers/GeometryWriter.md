# API Reference — `dna/layers/GeometryWriter`

---

<!-- ink:api name="GeometryWriter" module="dna/layers/GeometryWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class GeometryWriter : public virtual DefinitionWriter`

Write geometry data (meshes, vertex positions, normals, texture coordinates, and layouts) into a rig DNA structure.

### When to use this

Use this interface when you need to populate or replace the mesh geometry layer of a DNA asset during a write pass. Pair with `GeometryReader` to read geometry back after writing, or combine multiple `set*` calls to progressively build up mesh data before committing.

### Method groups

| Group | Methods |
|-------|---------|
| Mesh management | `clearMeshes`, `deleteMesh` |
| Vertex positions | `setVertexPositions` |
| Texture coordinates | `setVertexTextureCoordinates` |
| Vertex normals | `setVertexNormals` |
| Vertex layouts | `setVertexLayouts` |

### Example

```cpp
// Obtain a Writer (which also implements GeometryWriter) via factory
dna::Writer* writer = dna::Writer::create(stream, memRes);

// Set vertex positions for mesh 0
std::vector<dna::Position> positions = { {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} };
writer->setVertexPositions(0, positions.data(), static_cast<std::uint32_t>(positions.size()));

// Set normals for mesh 0
std::vector<dna::Normal> normals = { {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} };
writer->setVertexNormals(0, normals.data(), static_cast<std::uint32_t>(normals.size()));

// Set texture coordinates for mesh 0
std::vector<dna::TextureCoordinate> uvs = { {0.5f, 0.5f}, {1.0f, 0.0f} };
writer->setVertexTextureCoordinates(0, uvs.data(), static_cast<std::uint32_t>(uvs.size()));

// Remove a mesh no longer needed
writer->deleteMesh(2);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `meshIndex` | `std::uint16_t` | required — zero-indexed position of the target mesh. Must be less than `getMeshCount` for `deleteMesh`; implicitly resizes storage for `set*` methods. |
| `positions` | `const Position*` | required — source buffer of `Position` values to copy into the mesh. |
| `textureCoordinates` | `const TextureCoordinate*` | required — source buffer of `TextureCoordinate` values to copy into the mesh. |
| `normals` | `const Normal*` | required — source buffer of `Normal` values to copy into the mesh. |
| `layouts` | `const VertexLayout*` | required — source buffer of `VertexLayout` values to copy into the mesh. |
| `count` | `std::uint32_t` | required — number of elements to copy from the corresponding source buffer. |

### Constraints

- `meshIndex` passed to `deleteMesh` must be less than the value returned by `getMeshCount`.
- For all `set*` methods, mesh storage is implicitly resized to accommodate the inferred mesh count from `meshIndex` — no explicit pre-allocation required.

### Watch out for

- Do not inherit directly from `GeometryWriter`. Implementors must inherit from `Writer` itself; this interface is a write-layer mixin only.

<!-- ink:api-end name="GeometryWriter" -->
