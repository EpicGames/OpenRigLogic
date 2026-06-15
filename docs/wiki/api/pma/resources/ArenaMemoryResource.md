# API Reference — `pma/resources/ArenaMemoryResource`

---

<!-- ink:api name="ArenaMemoryResource" module="pma/resources/ArenaMemoryResource" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `ArenaMemoryResource`

Allocate memory from a growing pool of preallocated regions, avoiding per-request calls to the system allocator. Use this when many objects share the same lifetime and you want to free them all at once by destroying the arena.

### When to use this

Reach for `ArenaMemoryResource` when you have a burst of allocations that all become invalid at the same point — for example, per-frame scratch buffers, per-request parsing state, or loading-time temporaries. Individual `deallocate` calls are no-ops; reclamation happens only when the arena is destroyed. Use a general-purpose `MemoryResource` instead when objects have independent, unpredictable lifetimes.

### Example

```cpp
// Upstream allocator (e.g. the default system allocator)
pma::MemoryResource* upstream = pma::getAllocator();

// Arena with 64 KiB initial region, 256 KiB subsequent regions, 2x growth
pma::ArenaMemoryResource arena{65536u, 262144u, 2.0f, upstream};

// Serve individual allocations — no system-allocator calls until a new region is needed
void* buf = arena.allocate(1024u, alignof(std::max_align_t));

// No-op: individual frees do nothing; all memory reclaimed when arena destructs
arena.deallocate(buf, 1024u, alignof(std::max_align_t));

// Fixed-size regions (no growth): all regions are 128 KiB
pma::ArenaMemoryResource fixed{131072u, upstream};
```

### Parameters

The class provides three constructor overloads; all require an `upstream` allocator.

| Name | Type | Description |
|------|------|-------------|
| `initialSize` | `std::size_t` | optional (overload 1 only) — size in bytes of the first region; subsequent regions use `regionSize`. |
| `regionSize` | `std::size_t` | required — size in bytes of each additionally allocated region (and the initial region when `initialSize` is absent). |
| `growthFactor` | `float` | optional (overloads 1–2) — multiplier applied to each subsequent region relative to the previous one. `1.0` means all regions are the same size. Omit to default to `1.0` (no growth). |
| `upstream` | `MemoryResource*` | required — backing allocator used to obtain each region from the OS or a parent pool. |

### Watch out for

- `deallocate` is a no-op. Calling it does not free any memory. All memory held by the arena is released only when the `ArenaMemoryResource` object is destroyed or goes out of scope. Do not rely on individual frees for resource management.
- Copy construction and copy assignment are deleted. `ArenaMemoryResource` is move-only — transfer ownership with `std::move` rather than copying.

### Constraints

- Region allocation sequence follows the geometric series: `{initialSize, regionSize, regionSize × growthFactor, regionSize × growthFactor², …}`. When `initialSize` is omitted, the first region is also `regionSize`.
- The `upstream` pointer must remain valid for the entire lifetime of the `ArenaMemoryResource`.

<!-- ink:api-end name="ArenaMemoryResource" -->
