# API Reference — `dna/layers/MachineLearnedBehaviorReader`

---

<!-- ink:api name="MachineLearnedBehaviorReader" module="dna/layers/MachineLearnedBehaviorReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class MachineLearnedBehaviorReader : public virtual DefinitionReader`

Query neural network topology and ML control data from a loaded rig asset.

### When to use this

Reach for this interface when you need to inspect the machine-learned behavior layer of a DNA rig — iterating over ML controls by name, resolving which neural networks are active at a given LOD, or mapping mesh regions to their associated network indices. Use `MachineLearnedBehaviorWriter` when you need to author or modify this data rather than read it.

### Method groups

| Group | Methods |
|---|---|
| ML Controls | `getMLControlCount`, `getMLControlName` |
| Neural Networks | `getNeuralNetworkCount`, `getNeuralNetworkIndexListCount`, `getNeuralNetworkIndicesForLOD`, `getNeuralNetworkIndicesForMeshRegion` |
| Mesh Regions | `getMeshRegionCount`, `getMeshRegionName` |

### Example

```cpp
// Assume `reader` is a concrete Reader obtained via BinaryStreamReader::create()

// Iterate all ML control names
for (std::uint16_t i = 0u; i < reader->getMLControlCount(); ++i) {
    StringView name = reader->getMLControlName(i);
    // use name ...
}

// Resolve neural networks active at LOD 0
ConstArrayView<std::uint16_t> lod0Nets = reader->getNeuralNetworkIndicesForLOD(0u);

// Map mesh regions to their neural networks (mesh index 0)
for (std::uint16_t r = 0u; r < reader->getMeshRegionCount(0u); ++r) {
    StringView regionName = reader->getMeshRegionName(0u, r);
    ConstArrayView<std::uint16_t> nets = reader->getNeuralNetworkIndicesForMeshRegion(0u, r);
}
```

### Watch out for

- Do not subclass `MachineLearnedBehaviorReader` directly in implementations — inherit from `Reader` instead. This class is an interface layer, not the implementation base.
- Every index parameter (`index`, `lod`, `meshIndex`, `regionIndex`) must be less than the value returned by its corresponding count method. Passing an out-of-range index has undefined behavior; validate with `getMLControlCount`, `getLODCount`, `getMeshCount`, and `getMeshRegionCount` first.
- `getNeuralNetworkIndexListCount()` is meaningful only when used alongside `MachineLearnedBehaviorWriter`. During read-only traversal its value does not correspond to the count of LOD or mesh-region index lists — use `getNeuralNetworkIndicesForLOD` or `getNeuralNetworkIndicesForMeshRegion` to enumerate those.

<!-- ink:api-end name="MachineLearnedBehaviorReader" -->
