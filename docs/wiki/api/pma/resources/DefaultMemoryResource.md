# API Reference — `pma/resources/DefaultMemoryResource`

---

<!-- ink:api name="DefaultMemoryResource" module="pma/resources/DefaultMemoryResource" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `class DefaultMemoryResource : public MemoryResource`

A `MemoryResource` that delegates directly to `malloc`/`free`.

### Why this exists

`DefaultMemoryResource` gives `pma` containers a `MemoryResource`-compatible fallback that behaves exactly like ordinary heap allocation, so code that doesn't need arena pooling or alignment guarantees can still participate in the same allocator abstraction as code that does.

### Relationships

- `MemoryResource` — *base interface `DefaultMemoryResource` implements.*
- `AlignedMemoryResource` / `ArenaMemoryResource` — *sibling resources with stricter alignment or pooled-allocation behavior.*

<!-- ink:api-end name="DefaultMemoryResource" -->
