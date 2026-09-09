# API Reference — `dna/layers/MachineLearnedBehaviorWriter`

---

<!-- ink:api name="MachineLearnedBehaviorWriter" module="dna/layers/MachineLearnedBehaviorWriter" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class MachineLearnedBehaviorWriter : public virtual DefinitionWriter`

Write-only accessors for the neural network data associated with a rig — defining ML control names, the neural-network index lists, which networks run at each LOD, and mesh-region associations.

### When to use this

Use this when building or editing a rig's neural-network configuration: naming ML controls, assigning neural networks to LODs, and associating regions of a mesh with the networks that drive them. Implementors should inherit from `Writer` itself, not this class directly.

### Method groups

| Group | Methods |
|-------|---------|
| ML controls | clearMLControlNames, setMLControlName |
| Neural networks | clearNeuralNetworks, clearNeuralNetworkIndices, setNeuralNetworkIndices |
| LOD mapping | clearLODNeuralNetworkMappings, setLODNeuralNetworkMapping |
| Mesh regions | clearMeshRegionNames (all meshes and per-mesh overload) |

### Example

```cpp
// writer implements Writer, which composes MachineLearnedBehaviorWriter
writer->setMLControlName(0u, "jawOpen");
const std::uint16_t netIndices[] = {0u, 1u, 2u};
writer->setNeuralNetworkIndices(0u, netIndices, 3u);
writer->setLODNeuralNetworkMapping(0u, 0u); // LOD 0 uses index list 0
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `index` | `std::uint16_t` | required — position of the ML control name or neural network index list; storage resizes implicitly to fit. |
| `name` | `const char*` | required — null-terminated control name string; copied internally (extra allocation). |
| `netIndices` | `const std::uint16_t*` | required — source address of neural network indices to copy. |
| `count` | `std::uint16_t` | required — number of neural network indices to copy. |
| `lod` | `std::uint16_t` | required — level of detail being associated with a neural network index list. |
| `meshIndex` | `std::uint16_t` | required — mesh position; must be less than `DefinitionReader::getMeshCount`. |

### Watch out for

- `index` in `setNeuralNetworkIndices` denotes the row of a 2D matrix of neural network indices — the whole list's position, not an individual element's.
- Name and index storage are implicitly resized based on the index passed, so writes can be made out of order without pre-sizing.
- String arguments to `setMLControlName` and `setMeshRegionName` are copied immediately; the caller's buffer can be freed after the call returns.

### Constraints

- `deleteNeuralNetwork(netIndex)` and `clearNeuralNetworkLayers(netIndex)`: `netIndex` must be less than the value returned by `MachineLearnedBehaviorReader::getNeuralNetworkCount()`.

<!-- ink:api-end name="MachineLearnedBehaviorWriter" -->
