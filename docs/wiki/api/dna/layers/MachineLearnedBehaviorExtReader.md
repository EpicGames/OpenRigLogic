# API Reference — `dna/layers/MachineLearnedBehaviorExtReader`

---

<!-- ink:api name="MachineLearnedBehaviorExtReader" module="dna/layers/MachineLearnedBehaviorExtReader" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class MachineLearnedBehaviorExtReader : public virtual MachineLearnedBehaviorReader`

Read-only accessors to the neural network extension data associated with a rig — the per-model operation graph (operation sets, operation types, static parameters, and dependency indices) that `MachineLearnedBehaviorReader` itself does not expose.

### When to use this

Use this when you need to evaluate or inspect the actual ML operation graph for a rig — which operations run in which set, what type each operation is, and what it depends on. Implementors should inherit from `Reader` itself, not from this class directly, since it is meant to be composed into the full reader interface.

### Method groups

| Group | Methods |
|-------|---------|
| ML type / operation set counts | getMLTypeCount, getMLOperationSetCount, getMLOperationCount |
| Operation data | getMLOperationType, getMLOperationParameters |
| Operation dependencies | getMLOperationDependencyOperationIndices (and related dependency-index accessors) |

### Example

```cpp
// reader implements Reader, which composes MachineLearnedBehaviorExtReader
const auto mlTypeCount = reader->getMLTypeCount();
for (std::uint16_t mlType = 0u; mlType < mlTypeCount; ++mlType) {
    const auto setCount = reader->getMLOperationSetCount(mlType);
    for (std::uint16_t setIdx = 0u; setIdx < setCount; ++setIdx) {
        const auto opCount = reader->getMLOperationCount(mlType, setIdx);
        for (std::uint16_t opIdx = 0u; opIdx < opCount; ++opIdx) {
            auto opType = reader->getMLOperationType(mlType, setIdx, opIdx);
            // dispatch evaluation based on opType
        }
    }
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `mlTypeIndex` | `std::uint16_t` | required — position in the zero-indexed array of ML models; must be less than `getMLTypeCount`. |
| `mlOperationSetIndex` | `std::uint16_t` | required — position of the operation set within the selected ML model; must be less than `getMLOperationSetCount`. |
| `mlOperationIndex` | `std::uint16_t` | required — position of the operation within the selected set; must be less than `getMLOperationCount`. |

### Returns

`std::uint16_t` / `MachineLearnedBehaviorOperationType` / `ConstArrayView<std::uint32_t>` — counts, the operation's type, or a view over its static parameters/dependency indices, depending on the accessor called.

### Watch out for

- Operations within a single operation set are safe to execute in parallel, but operation sets themselves must be executed sequentially, one after another.
- Each subsequent operation set can access the results of earlier sets, which is the basis for the dependency-index accessors.
- Index bounds are not checked automatically: `mlTypeIndex` must be less than `getMLTypeCount()`, `mlOperationSetIndex` less than `getMLOperationSetCount(mlTypeIndex)`, and `mlOperationIndex` less than `getMLOperationCount(mlTypeIndex, mlOperationSetIndex)`. Passing an out-of-range index produces undefined behavior.
- Dependency indices from `getMLOperationDependencyOperationSetIndices` and `getMLOperationDependencyOperationIndices` form paired lists: element `i` from each list together identify one `(operation-set, operation)` dependency pair. Process both arrays together under the same index.

<!-- ink:api-end name="MachineLearnedBehaviorExtReader" -->
