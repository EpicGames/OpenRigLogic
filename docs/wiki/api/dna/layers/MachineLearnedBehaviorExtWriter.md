# API Reference — `dna/layers/MachineLearnedBehaviorExtWriter`

---

<!-- ink:api name="MachineLearnedBehaviorExtWriter" module="dna/layers/MachineLearnedBehaviorExtWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class MachineLearnedBehaviorExtWriter : public virtual MachineLearnedBehaviorWriter`

Populate and clear the ML extension operation data stored in a rig's DNA layer.

### When to use this

Use this interface when implementing a custom `Writer` subclass that needs to write ML extended behavior operation sets into a DNA asset. It provides the full mutation surface for operation type assignment, static parameter injection, and dependency graph construction across the three-level hierarchy of ML type → operation set → operation.

### Watch out for

- Do not inherit from `MachineLearnedBehaviorExtWriter` directly. The `@warning` on the class requires that implementors inherit from `Writer` itself. This class exists solely to group the extended ML write accessors; direct inheritance bypasses the full writer contract.

### Method groups

| Group | Methods |
|---|---|
| Bulk clear | `clearMLExtData`, `clearMLOperationSets`, `clearMLOperations` |
| Set operation data | `setMLOperationType`, `setMLOperationParameters` |

### Example

```cpp
// Concrete Writer subclass (inherit from Writer, not this class directly)
class MyRigWriter : public Writer {
public:
    void writeMLExtensions(MachineLearnedBehaviorExtWriter* w,
                           std::uint16_t mlTypeIndex) {
        // Clear any prior state for this ML type before repopulating
        w->clearMLOperationSets(mlTypeIndex);

        // Assign the operation type for operation 0 in set 0
        w->setMLOperationType(mlTypeIndex, /*setIndex=*/0, /*opIndex=*/0,
                              MachineLearnedBehaviorOperationType::Multiply);

        // Attach static parameters to the same operation
        const std::uint32_t params[] = {1024u, 2u};
        w->setMLOperationParameters(mlTypeIndex, 0, 0, params, 2);
    }
};
```

### Parameters

All mutating methods share the same three-level index scheme:

| Name | Type | Description |
|---|---|---|
| `mlTypeIndex` | `std::uint16_t` | required — zero-based index of the ML model in the rig's ML model array |
| `mlOperationSetIndex` | `std::uint16_t` | required — zero-based index of the operation set within the selected ML model |
| `mlOperationIndex` | `std::uint16_t` | required — zero-based index of the individual operation within the selected set |
| `operationType` | `MachineLearnedBehaviorOperationType` | required (`setMLOperationType` only) — the operation variant to assign |
| `parameters` | `const std::uint32_t*` | required (`setMLOperationParameters` only) — pointer to the static parameter array to copy |
| `count` | `std::uint16_t` | required (`setMLOperationParameters` only) — number of elements to copy from `parameters` |

### Constraints

- Storage at all three levels is implicitly resized when a `set*` call references an index beyond the current array bounds. No pre-allocation step is required, but callers should write indices in a predictable order to avoid sparse allocations.
- Clearing is hierarchical: `clearMLExtData` removes all ML extension data; `clearMLOperationSets` removes all sets for one ML type; `clearMLOperations` removes all operations within one set.

<!-- ink:api-end name="MachineLearnedBehaviorExtWriter" -->
