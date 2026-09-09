# API Reference — `pma/resources/ArenaMemoryResource`

---

<!-- ink:api name="ArenaMemoryResource" module="pma/resources/ArenaMemoryResource" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `class ArenaMemoryResource : public MemoryResource`

Serves allocations from a preallocated memory region instead of hitting the upstream allocator on every request.

### Why this exists

Arena allocation amortizes the cost of many small allocations into a few large upstream requests, and lets a whole set of related allocations be released together implicitly (deallocate is a no-op — the arena frees everything when it itself is destroyed). When the active region runs out of space, an additional region is allocated and scaled by `growthFactor`, so the arena grows without requiring the caller to size it perfectly up front.

### Construction

```cpp
// initial region sized separately from subsequent growth regions, 1.5x growth per new region
pma::ArenaMemoryResource arena{/*initialSize=*/4096, /*regionSize=*/1024, /*growthFactor=*/1.5f, upstream};

// simpler form: same regionSize used for initial and all subsequent regions, no growth
pma::ArenaMemoryResource fixedArena{/*regionSize=*/1024, upstream};
```

### Relationships

- `MemoryResource` — *base interface `ArenaMemoryResource` implements.*
- `ScopedPtr` / `FactoryDestroy` — *used internally to manage the arena's private `Impl`.*

### Constraints

- Not copyable — copy constructor and copy assignment are deleted.
- Move-only: supports move construction and move assignment.
- `deallocate` is a no-op; regions are only freed when the arena itself is destroyed.
- Region allocation sequence follows the geometric series: `{initialSize, regionSize, regionSize × growthFactor, regionSize × growthFactor², …}`. When `initialSize` is omitted, the first region is also `regionSize`.
- The `upstream` pointer must remain valid for the entire lifetime of the `ArenaMemoryResource`.

### Watch out for

- `deallocate` is a no-op. Calling it does not free any memory. All memory held by the arena is released only when the `ArenaMemoryResource` object is destroyed or goes out of scope. Do not rely on individual frees for resource management.
- Copy construction and copy assignment are deleted. `ArenaMemoryResource` is move-only — transfer ownership with `std::move` rather than copying.

<!-- ink:api-end name="ArenaMemoryResource" -->
