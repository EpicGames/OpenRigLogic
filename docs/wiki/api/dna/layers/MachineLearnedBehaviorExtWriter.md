# API Reference — `dna/layers/MachineLearnedBehaviorExtWriter`

---

<!-- ink:api name="MachineLearnedBehaviorExtWriter" module="dna/layers/MachineLearnedBehaviorExtWriter" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class MachineLearnedBehaviorExtWriter : public virtual MachineLearnedBehaviorWriter`

Write-only accessors for the operation instance data associated with a rig — building up the same per-model operation graph that `MachineLearnedBehaviorExtReader` exposes for reading.

### When to use this

Use this when constructing or editing the ML operation graph for a rig: defining operation types, their static parameters, and the dependencies between operation sets. Implementors should inherit from `Writer` itself, not from this class directly.

### Method groups

| Group | Methods |
|-------|---------|
| Clear | clearMLExtData, clearMLOperationSets, clearMLOperations |
| Operation data | setMLOperationType, setMLOperationParameters |
| Operation dependencies | dependency-index setters (paired with MachineLearnedBehaviorExtReader's dependency accessors) |

### Example

```cpp
// writer implements Writer, which composes MachineLearnedBehaviorExtWriter
writer->clearMLOperations(0u, 0u);
writer->setMLOperationType(0u, 0u, 0u, dna::MachineLearnedBehaviorOperationType::MLP);
const std::uint32_t params[] = {0u, 1u, 2u};
writer->setMLOperationParameters(0u, 0u, 0u, params, 3u);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `mlTypeIndex` | `std::uint16_t` | required — position in the zero-indexed array of ML models. |
| `mlOperationSetIndex` | `std::uint16_t` | required — position of the operation set within the selected ML model. |
| `mlOperationIndex` | `std::uint16_t` | required — position of the operation within the selected set. |
| `operationType` | `MachineLearnedBehaviorOperationType` | required — the type to assign to the operation. |
| `parameters` | `const std::uint32_t*` | required — source address from which static parameters are copied. |
| `count` | `std::uint16_t` | required — number of parameters to copy. |

### Watch out for

- Storage for operation sets/operations is implicitly resized when needed, inferred from the index passed — you don't need to pre-size it yourself.
- Do not inherit from `MachineLearnedBehaviorExtWriter` directly. The `@warning` on the class requires that implementors inherit from `Writer` itself. This class exists solely to group the extended ML write accessors; direct inheritance bypasses the full writer contract.

### Constraints

- Storage at all three levels is implicitly resized when a `set*` call references an index beyond the current array bounds. No pre-allocation step is required, but callers should write indices in a predictable order to avoid sparse allocations.

<!-- ink:api-end name="MachineLearnedBehaviorExtWriter" -->
