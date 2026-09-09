# API Reference — `dna/layers/MachineLearnedBehaviorReader`

---

<!-- ink:api name="MachineLearnedBehaviorReader" module="dna/layers/MachineLearnedBehaviorReader" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class MachineLearnedBehaviorReader : public virtual DefinitionReader`

Read-only accessors to the neural network data associated with a rig — ML control names, neural network counts, which networks are active at each LOD, and which networks drive which mesh region.

### When to use this

Use this to discover how many neural networks a rig has, which ones are relevant at a given level of detail, and which mesh regions they affect. Implementors should inherit from `Reader` itself, not this class directly.

### Method groups

| Group | Methods |
|-------|---------|
| ML controls | getMLControlCount, getMLControlName |
| Neural networks | getNeuralNetworkCount, getNeuralNetworkIndexListCount, getNeuralNetworkIndicesForLOD |
| Mesh regions | getMeshRegionCount, getMeshRegionName, getNeuralNetworkIndicesForMeshRegion |

### Example

```cpp
// reader implements Reader, which composes MachineLearnedBehaviorReader
const auto lod = 0u;
for (auto netIdx : reader->getNeuralNetworkIndicesForLOD(lod)) {
    // evaluate neural network at netIdx
}

const auto meshIdx = 0u;
for (std::uint16_t r = 0u; r < reader->getMeshRegionCount(meshIdx); ++r) {
    auto regionName = reader->getMeshRegionName(meshIdx, r);
    auto nets = reader->getNeuralNetworkIndicesForMeshRegion(meshIdx, r);
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `index` | `std::uint16_t` | required — position of the ML control name; must be less than `getMLControlCount`. |
| `lod` | `std::uint16_t` | required — level of detail; must be less than `DescriptorReader::getLODCount`. |
| `meshIndex` | `std::uint16_t` | required — mesh position; must be less than `DefinitionReader::getMeshCount`. |
| `regionIndex` | `std::uint16_t` | required — region position; must be less than `getMeshRegionCount`. |

### Returns

`std::uint16_t` / `StringView` / `ConstArrayView<std::uint16_t>` — counts, name strings, or index lists depending on the accessor called.

### Watch out for

- `getNeuralNetworkIndexListCount` is only useful in the context of `MachineLearnedBehaviorWriter` — reading it in isolation from `Reader` has limited meaning.
- Do not subclass `MachineLearnedBehaviorReader` directly in implementations — inherit from `Reader` instead. This class is an interface layer, not the implementation base.
- Every index parameter (`index`, `lod`, `meshIndex`, `regionIndex`) must be less than the value returned by its corresponding count method. Passing an out-of-range index has undefined behavior; validate with `getMLControlCount`, `getLODCount`, `getMeshCount`, and `getMeshRegionCount` first.
- `getNeuralNetworkIndexListCount()` is meaningful only when used alongside `MachineLearnedBehaviorWriter`. During read-only traversal its value does not correspond to the count of LOD or mesh-region index lists — use `getNeuralNetworkIndicesForLOD` or `getNeuralNetworkIndicesForMeshRegion` to enumerate those.

<!-- ink:api-end name="MachineLearnedBehaviorReader" -->
