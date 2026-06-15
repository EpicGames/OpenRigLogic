# API Reference — `dna/layers/MachineLearnedBehaviorExtReader`

---

<!-- ink:api name="MachineLearnedBehaviorExtReader" module="dna/layers/MachineLearnedBehaviorExtReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `MachineLearnedBehaviorExtReader`

Access the neural network execution graph for a rig — enumerate ML model types, operation sets, and per-operation parameters and dependencies.

### When to use this

Use this interface (obtained through `Reader`) when you need to traverse the full ML operation DAG: iterating model types, walking sequential operation sets, and reading per-operation parameters or dependency chains. Use `MachineLearnedBehaviorReader` instead when you only need higher-level ML behavior data and do not need to inspect individual operations or their dependencies.

### Method groups

| Group | Methods |
|-------|---------|
| Model structure | `getMLTypeCount`, `getMLOperationSetCount`, `getMLOperationCount` |
| Operation details | `getMLOperationType`, `getMLOperationParameters` |
| Operation dependencies | `getMLOperationDependencyOperationSetIndices`, `getMLOperationDependencyOperationIndices` |

### Example

```cpp
// Traverse all ML models and their operation sets
for (std::uint16_t t = 0; t < reader->getMLTypeCount(); ++t) {
    for (std::uint16_t s = 0; s < reader->getMLOperationSetCount(t); ++s) {
        // Operations within a set are safe to run in parallel
        for (std::uint16_t op = 0; op < reader->getMLOperationCount(t, s); ++op) {
            auto opType   = reader->getMLOperationType(t, s, op);
            auto opParams = reader->getMLOperationParameters(t, s, op);
            // Schedule opType with opParams; check dependencies before dispatch
        }
    }
}
```

### Watch out for

- Index bounds are not checked automatically: `mlTypeIndex` must be less than `getMLTypeCount()`, `mlOperationSetIndex` less than `getMLOperationSetCount(mlTypeIndex)`, and `mlOperationIndex` less than `getMLOperationCount(mlTypeIndex, mlOperationSetIndex)`. Passing an out-of-range index produces undefined behavior.
- Operations within a single operation set are safe to execute in parallel, but operation sets themselves must be evaluated sequentially. Reversing this ordering produces incorrect deformation results.
- Do not inherit directly from this class — implementors must inherit from `Reader` instead, as stated in the class-level warning.
- Dependency indices from `getMLOperationDependencyOperationSetIndices` and `getMLOperationDependencyOperationIndices` form paired lists: element `i` from each list together identify one `(operation-set, operation)` dependency pair. Process both arrays together under the same index.

<!-- ink:api-end name="MachineLearnedBehaviorExtReader" -->
