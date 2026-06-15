# API Reference — `dna/layers/MachineLearnedBehaviorWriter`

---

<!-- ink:api name="MachineLearnedBehaviorWriter" module="dna/layers/MachineLearnedBehaviorWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class DNAAPI MachineLearnedBehaviorWriter : public virtual DefinitionWriter`

Write, clear, and organize neural network data — ML control names, network topology, layer weights, and LOD mappings — for a character rig.

### When to use this

Use this interface when constructing or modifying the machine-learned behavior layer of a DNA file: populating neural network definitions, assigning input/output control indices, setting per-layer activation functions, and mapping networks to LODs or mesh regions. Do not inherit from this class directly — inherit from `Writer`, which aggregates all layer writers including this one.

### Method groups

| Group | Methods |
|-------|---------|
| ML Controls | `clearMLControlNames`, `setMLControlName` |
| Neural Networks | `clearNeuralNetworks`, `deleteNeuralNetwork` |
| Neural Network Indices | `clearNeuralNetworkIndices`, `setNeuralNetworkIndices`, `clearNeuralNetworkIndicesPerMeshRegion`, `setNeuralNetworkIndicesForMeshRegion` |
| LOD Mapping | `clearLODNeuralNetworkMappings`, `setLODNeuralNetworkMapping` |
| Mesh Regions | `clearMeshRegionNames`, `setMeshRegionName` |
| Neural Network I/O | `setNeuralNetworkInputIndices`, `setNeuralNetworkOutputIndices` |
| Neural Network Layers | `clearNeuralNetworkLayers`, `setNeuralNetworkLayerActivationFunction`, `setNeuralNetworkLayerActivationFunctionParameters`, `setNeuralNetworkLayerBiases`, `setNeuralNetworkLayerWeights` |

### Example

```cpp
// Populate ML control names and neural network structure
writer->clearMLControlNames();
writer->setMLControlName(0u, "jawOpen");

// Assign networks 0 and 1 to LOD 0
uint16_t netIndices[] = {0u, 1u};
writer->setNeuralNetworkIndices(0u, netIndices, 2u);
writer->setLODNeuralNetworkMapping(0u, 0u);

// Wire input/output controls for network 0
uint16_t inputs[] = {4u, 5u, 6u};
writer->setNeuralNetworkInputIndices(0u, inputs, 3u);
uint16_t outputs[] = {12u, 13u};
writer->setNeuralNetworkOutputIndices(0u, outputs, 2u);
```

### Constraints

- Do not inherit from this class directly. Implementors must inherit from `Writer`.
- `clearMeshRegionNames(meshIndex)`: `meshIndex` must be less than the value returned by `DefinitionReader::getMeshCount()`.
- `deleteNeuralNetwork(netIndex)` and `clearNeuralNetworkLayers(netIndex)`: `netIndex` must be less than the value returned by `MachineLearnedBehaviorReader::getNeuralNetworkCount()`.
- Storage arrays (control names, neural network indices, regions) are implicitly resized when the specified index exceeds the current capacity.
- Passed-in `const char*` name strings are copied on each call, which involves an additional allocation per call.

### Watch out for

- `setNeuralNetworkIndices` treats `index` as a row index in a 2D matrix of index lists — it is not the index of an individual neural network. Passing the network index directly here will misalign the LOD mapping set by `setLODNeuralNetworkMapping`.
- String arguments to `setMLControlName` and `setMeshRegionName` are copied immediately; the caller's buffer can be freed after the call returns.

<!-- ink:api-end name="MachineLearnedBehaviorWriter" -->
