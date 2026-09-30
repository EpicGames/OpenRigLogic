# API Reference — `dna/layers/DefinitionReader`

---

<!-- ink:api name="DefinitionReader" module="dna/layers/DefinitionReader" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->
## `class DNAAPI DefinitionReader : public DescriptorReader`

Read the rig's static data: GUI and raw control names, joint names, per-LOD joint indices and the joint hierarchy.

Read-only interface to the rig's static definition data — controls, joints, blend shape channels, animated maps, meshes, and neutral joint transforms.

### When to use this

Use this when you need to query the structural definition of a DNA rig without modifying it. Reach for `DefinitionWriter` when you need to write or replace any of these same attributes.

### Method groups

| Group | Methods |
|-------|---------|
| GUI Controls | `getGUIControlCount`, `getGUIControlName` |
| Raw Controls | `getRawControlCount`, `getRawControlName` |
| Joints | `getJointCount`, `getJointName`, `getJointIndexListCount`, `getJointIndicesForLOD`, `getJointParentIndex` |
| Blend Shape Channels | `getBlendShapeChannelCount`, `getBlendShapeChannelName`, `getBlendShapeChannelIndexListCount`, `getBlendShapeChannelIndicesForLOD` |
| Animated Maps | `getAnimatedMapCount`, `getAnimatedMapName`, `getAnimatedMapIndexListCount`, `getAnimatedMapIndicesForLOD` |
| Meshes | `getMeshCount`, `getMeshName`, `getMeshIndexListCount`, `getMeshIndicesForLOD` |
| Mesh-Blend Shape Mappings | `getMeshBlendShapeChannelMappingCount`, `getMeshBlendShapeChannelMapping`, `getMeshBlendShapeChannelMappingIndicesForLOD` |
| Neutral Joint Transforms | `getNeutralJointTranslation`, `getNeutralJointTranslationXs`, `getNeutralJointTranslationYs`, `getNeutralJointTranslationZs`, `getNeutralJointRotation`, `getNeutralJointRotationXs`, `getNeutralJointRotationYs`, `getNeutralJointRotationZs` |

### Example

```cpp
// Iterate GUI control names
std::uint16_t guiCount = def->getGUIControlCount();
for (std::uint16_t i = 0; i < guiCount; ++i) {
    StringView name = def->getGUIControlName(i);  // valid until next DNA call
}

// Collect LOD-0 joint names using the joint index list
ConstArrayView<std::uint16_t> lod0Joints = def->getJointIndicesForLOD(0);
for (std::uint16_t idx : lod0Joints) {
    StringView jointName = def->getJointName(idx);
    std::uint16_t parentIdx = def->getJointParentIndex(idx);  // same as idx for root
}

// Resolve mesh-blend shape mappings at LOD 1
ConstArrayView<std::uint16_t> mappingIndices =
    def->getMeshBlendShapeChannelMappingIndicesForLOD(1);
for (std::uint16_t mi : mappingIndices) {
    MeshBlendShapeChannelMapping m = def->getMeshBlendShapeChannelMapping(mi);
    // m.meshIndex, m.blendShapeChannelIndex
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `index` | `std::uint16_t` | required. Position in the zero-indexed name array. Must be less than the matching count. |
| `lod` | `std::uint16_t` | required. Level of detail. Must be less than `getLODCount`. |

### Returns

`StringView` over a name, or `ConstArrayView<std::uint16_t>` over joint indices.

### Watch out for

- The joint hierarchy can be rebuilt from the parent-index array. For names [A..I] and hierarchy [0, 0, 0, 1, 1, 4, 2, 6, 2], A is the root, B and C are its children, and so on.
- Do not inherit from `DefinitionReader` directly — inherit from `Reader` instead. Inheriting from `DefinitionReader` bypasses required `Reader` infrastructure and produces an incomplete implementation.
- Every index-based accessor (`getGUIControlName`, `getJointName`, `getBlendShapeChannelName`, etc.) requires the index to be strictly less than its corresponding count return value. Passing an out-of-range index is undefined behavior; always guard with the count accessor first.
- The `*IndexListCount` methods (`getJointIndexListCount`, `getBlendShapeChannelIndexListCount`, `getMeshIndexListCount`, `getAnimatedMapIndexListCount`) are meaningful only in the context of `DefinitionWriter`. Do not use them to iterate LOD-filtered data — use the corresponding `*IndicesForLOD` methods instead.
- `getJointParentIndex` returns `UINT16_MAX` for out-of-bounds requests, and returns the same index as the input for the root joint (index 0). Check for both sentinel values when traversing the joint hierarchy.
- Blend shape channel LOD indices from `getBlendShapeChannelIndicesForLOD` are not interchangeable with LOD values from `BehaviorReader::getBlendShapeChannelLODs`.
- The bulk scalar accessors (`getNeutralJointTranslationXs`, `getNeutralJointRotationYs`, etc.) are advanced, performance-critical APIs. For general use, prefer `getNeutralJointTranslation` and `getNeutralJointRotation`.
<!-- ink:api-end name="DefinitionReader" -->

<!-- ink:api name="MeshBlendShapeChannelMapping" module="dna/layers/DefinitionReader" last_commit="api_scan" updated="2026-09-30" api_kind="data_shape" -->
## `MeshBlendShapeChannelMapping`

Mapping that associates a blend shape channel with its mesh.

### Why this exists

Blend shape channels in the DNA layer are stored as a flat list with no intrinsic grouping by mesh. This struct provides the explicit pairing needed to reconstruct which channels belong to which mesh, enabling O(1) lookup of mesh ownership per channel. It is the element type returned by `getMeshBlendShapeChannelMapping` and indexed through `getMeshBlendShapeChannelMappingIndicesForLOD` when iterating LOD-filtered mappings.

A blend shape channel is identified by an index, and that index alone does not say which mesh it belongs to. This struct pairs the two indices so the association is explicit.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `meshIndex` | `std::uint16_t` | required. Index of the mesh. |
| `blendShapeChannelIndex` | `std::uint16_t` | required. Index of the blend shape channel belonging to that mesh. |

### Relationships

- `DefinitionReader` — *declared in the same header as the Reader layer for rig static data*
- `DefinitionReader` — produces values of this type via `getMeshBlendShapeChannelMapping` and `getMeshBlendShapeChannelMappingIndicesForLOD`
<!-- ink:api-end name="MeshBlendShapeChannelMapping" -->
