# API Reference — `pma/resources/AlignedMemoryResource`

---

<!-- ink:api name="AlignedMemoryResource" module="pma/resources/AlignedMemoryResource" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `class AlignedMemoryResource : public MemoryResource`

A `MemoryResource` that honors alignment requirements stricter than the default allocator provides.

### Why this exists

Some allocations (SIMD buffers, hardware-aligned data) need guarantees beyond what `malloc`/`new` provide by default. `AlignedMemoryResource` implements `MemoryResource::allocate`/`deallocate` so any `pma` container or `PolyAllocator` consumer can honor a specific alignment without special-casing its own allocation logic.

### Relationships

- `MemoryResource` — *base interface `AlignedMemoryResource` implements.*
- `DefaultMemoryResource` — *sibling resource that delegates to malloc/free without alignment guarantees.*

### Watch out for

- `deallocate` requires passing the **same** `size` and `alignment` values that were used in the corresponding `allocate` call. Mismatched values are undefined behavior — track these at the call site.

<!-- ink:api-end name="AlignedMemoryResource" -->
